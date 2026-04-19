#ifdef DQN_AVAILABLE

#include "DQNAgent.hpp"
#include "QAgent.hpp" // reuse nextCell, floodFill
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>

// ── Network ──────────────────────────────────────────────────────────────────

DQNNetImpl::DQNNetImpl()
{
    fc1 = register_module("fc1", torch::nn::Linear(INPUT_SIZE, 64));
    fc2 = register_module("fc2", torch::nn::Linear(64, 32));
    fc3 = register_module("fc3", torch::nn::Linear(32, 4));
}

torch::Tensor DQNNetImpl::forward(torch::Tensor x)
{
    x = torch::relu(fc1->forward(x));
    x = torch::relu(fc2->forward(x));
    return fc3->forward(x);
}

// ── Agent construction ───────────────────────────────────────────────────────

DQNAgent::DQNAgent()
    : policyNet_(),
      targetNet_(),
      optimizer_(policyNet_->parameters(), torch::optim::AdamOptions(LR)),
      rng_(std::random_device{}()),
      stepCount_(0),
      epsilon_(EPSILON_START)
{
    updateTargetNetwork();
    targetNet_->eval();
}

// ── State encoding ───────────────────────────────────────────────────────────

torch::Tensor DQNAgent::encodeState(const Game& game)
{
    static const char DIRS[4] = {'w', 's', 'a', 'd'};

    const int        border   = game.getBorder();
    const int        interior = border - 2;
    const Point&     head     = game.getHead();
    const Point&     food     = game.getFruit();
    char* const*     grid     = game.grid();
    const int        hx       = head.getX();
    const int        hy       = head.getY();

    float buf[20];

    // [0-3] danger 1 step ahead in each absolute direction
    // [4-7] danger 2 steps ahead in each absolute direction
    for (int d = 0; d < 4; ++d) {
        int nx = 0, ny = 0;
        QAgent::nextCell(hx, hy, DIRS[d], border, nx, ny);
        buf[d] = (grid[nx][ny] == 'O') ? 1.0F : 0.0F;

        int nx2 = 0, ny2 = 0;
        QAgent::nextCell(nx, ny, DIRS[d], border, nx2, ny2);
        buf[4 + d] = (grid[nx2][ny2] == 'O') ? 1.0F : 0.0F;
    }

    // [8-11] current direction one-hot
    const char dir = (game.getLastMove() != 0) ? game.getLastMove() : 'd';
    buf[8]  = (dir == 'w') ? 1.0F : 0.0F;
    buf[9]  = (dir == 's') ? 1.0F : 0.0F;
    buf[10] = (dir == 'a') ? 1.0F : 0.0F;
    buf[11] = (dir == 'd') ? 1.0F : 0.0F;

    // [12-13] normalised food delta (wrapping-aware)
    int dx = food.getX() - hx;
    int dy = food.getY() - hy;
    if (dx > interior / 2)  dx -= interior;
    if (dx < -interior / 2) dx += interior;
    if (dy > interior / 2)  dy -= interior;
    if (dy < -interior / 2) dy += interior;
    const float norm = static_cast<float>(interior / 2);
    buf[12] = static_cast<float>(dx) / norm;
    buf[13] = static_cast<float>(dy) / norm;

    // [14] normalised Manhattan distance to food
    buf[14] = static_cast<float>(std::abs(dx) + std::abs(dy)) / static_cast<float>(interior);

    // [15] normalised body length
    buf[15] = static_cast<float>(game.getLength()) / static_cast<float>(interior * interior);

    // [16-19] flood-fill safety ratio in each direction
    const float totalCells = static_cast<float>(interior * interior);
    for (int d = 0; d < 4; ++d) {
        int nx = 0, ny = 0;
        QAgent::nextCell(hx, hy, DIRS[d], border, nx, ny);
        if (grid[nx][ny] == 'O') {
            buf[16 + d] = 0.0F;
        } else {
            buf[16 + d] = static_cast<float>(QAgent::floodFill(game, nx, ny)) / totalCells;
        }
    }

    return torch::from_blob(buf, {1, 20}, torch::kFloat32).clone();
}

// ── Epsilon decay (episode-based, mirrors QAgent) ───────────────────────────

void DQNAgent::decayEpsilon(float decayRate)
{
    epsilon_ = std::max(EPSILON_END, epsilon_ * (1.0F - decayRate));
}

float DQNAgent::computeDecayRate(int episodes)
{
    assert(episodes > 0);
    return static_cast<float>(
        1.0 - std::pow(static_cast<double>(EPSILON_END), 1.0 / static_cast<double>(episodes)));
}

// ── Action selection ─────────────────────────────────────────────────────────

int DQNAgent::selectAction(const torch::Tensor& state, bool training)
{
    if (training) {
        std::uniform_real_distribution<float> dist(0.0F, 1.0F);
        if (dist(rng_) < getEpsilon())
            return std::uniform_int_distribution<int>(0, NUM_ACTIONS - 1)(rng_);
    }
    return greedyAction(state);
}

int DQNAgent::greedyAction(const torch::Tensor& state) const
{
    torch::NoGradGuard noGrad;
    auto qValues = policyNet_->forward(state);
    return qValues.argmax(1).item<int>();
}

// ── Replay and training ──────────────────────────────────────────────────────

void DQNAgent::step(const torch::Tensor& state, int action, float reward,
                    const torch::Tensor& nextState, bool done)
{
    if (static_cast<int>(replayBuffer_.size()) >= REPLAY_CAPACITY)
        replayBuffer_.pop_front();
    replayBuffer_.push_back({state.detach(), action, reward, nextState.detach(), done});

    ++stepCount_;

    if (static_cast<int>(replayBuffer_.size()) >= MIN_REPLAY && stepCount_ % TRAIN_EVERY == 0)
        trainBatch();

    if (stepCount_ % TARGET_UPDATE == 0)
        updateTargetNetwork();
}

void DQNAgent::trainBatch()
{
    const int bufSize = static_cast<int>(replayBuffer_.size());

    std::vector<int> indices(BATCH_SIZE);
    std::uniform_int_distribution<int> dist(0, bufSize - 1);
    for (int& idx : indices)
        idx = dist(rng_);

    std::vector<torch::Tensor> states, nextStates;
    std::vector<int64_t>       actions;
    std::vector<float>         rewards;
    std::vector<float>         dones;
    states.reserve(BATCH_SIZE);
    nextStates.reserve(BATCH_SIZE);
    actions.reserve(BATCH_SIZE);
    rewards.reserve(BATCH_SIZE);
    dones.reserve(BATCH_SIZE);

    for (const int idx : indices) {
        const Transition& t = replayBuffer_[static_cast<std::size_t>(idx)];
        states.push_back(t.state);
        nextStates.push_back(t.nextState);
        actions.push_back(static_cast<int64_t>(t.action));
        rewards.push_back(t.reward);
        dones.push_back(t.done ? 1.0F : 0.0F);
    }

    auto stateBatch     = torch::cat(states, 0);
    auto nextStateBatch = torch::cat(nextStates, 0);
    auto actionBatch    = torch::tensor(actions, torch::kLong).unsqueeze(1);
    auto rewardBatch    = torch::tensor(rewards, torch::kFloat32);
    auto doneBatch      = torch::tensor(dones, torch::kFloat32);

    auto currentQ = policyNet_->forward(stateBatch).gather(1, actionBatch).squeeze(1);

    // Double DQN: policy net selects best action, target net evaluates it.
    torch::Tensor nextQ;
    {
        torch::NoGradGuard noGrad;
        auto bestActions = policyNet_->forward(nextStateBatch).argmax(1, true); // [B,1]
        nextQ = targetNet_->forward(nextStateBatch).gather(1, bestActions).squeeze(1); // [B]
    }
    auto targetQ = rewardBatch + GAMMA * nextQ * (1.0F - doneBatch);

    auto loss = torch::smooth_l1_loss(currentQ, targetQ.detach());
    optimizer_.zero_grad();
    loss.backward();
    torch::nn::utils::clip_grad_norm_(policyNet_->parameters(), 1.0);
    optimizer_.step();
}

void DQNAgent::updateTargetNetwork()
{
    torch::NoGradGuard noGrad;
    auto policyParams = policyNet_->parameters();
    auto targetParams = targetNet_->parameters();
    for (std::size_t i = 0; i < policyParams.size(); ++i)
        targetParams[i].copy_(policyParams[i]);
}

// ── Persistence ──────────────────────────────────────────────────────────────

bool DQNAgent::save(const std::string& path) const
{
    try {
        torch::save(policyNet_, path);
        return true;
    } catch (...) {
        return false;
    }
}

bool DQNAgent::load(const std::string& path)
{
    try {
        torch::load(policyNet_, path);
        updateTargetNetwork();
        return true;
    } catch (...) {
        return false;
    }
}

#endif // DQN_AVAILABLE
