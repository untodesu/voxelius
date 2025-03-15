#include "server/pch.hh"
#include "server/worldgen.hh"

#include "shared/chunk.hh"
#include "shared/dimension.hh"
#include "shared/protocol.hh"
#include "shared/threading.hh"

#include "server/sessions.hh"

static emhash8::HashMap<Dimension *, emhash8::HashMap<chunk_pos, std::unordered_set<Session *>>> active_tasks;

class WorldgenTask final : public Task {
public:
    explicit WorldgenTask(Dimension *dimension, const chunk_pos &cpos);
    virtual ~WorldgenTask(void) = default;
    virtual void process(void) override;
    virtual void finalize(void) override;

private:
    Dimension *m_dimension;
    VoxelStorage m_voxels;
    chunk_pos m_cpos;
};

WorldgenTask::WorldgenTask(Dimension *dimension, const chunk_pos &cpos)
{
    m_dimension = dimension;
    m_voxels.fill(rand()); // trolling
    m_cpos = cpos;
}

void WorldgenTask::process(void)
{
    if(!m_dimension->generate(m_cpos, m_voxels)) {
        set_status(task_status::CANCELLED);
    }
}

void WorldgenTask::finalize(void)
{
    auto dim_tasks = active_tasks.find(m_dimension);

    if(dim_tasks == active_tasks.cend()) {
        // Normally this should never happen but
        // one can never be sure about anything
        // when that anything is threaded out
        return;
    }

    auto it = dim_tasks->second.find(m_cpos);

    if(it == dim_tasks->second.cend()) {
        // Normally this should never happen but
        // one can never be sure about anything
        // when that anything is threaded out
        return;
    }

    auto chunk = m_dimension->create_chunk(m_cpos);
    chunk->set_voxels(m_voxels);

    protocol::ChunkVoxels response;
    response.voxels = m_voxels;
    response.chunk = m_cpos;

    auto packet = protocol::encode(response);

    for(auto session : it->second) {
        if(session->peer) {
            // Respond with the voxels to every session
            // that has requested this specific chunk for this dimension
            enet_peer_send(session->peer, protocol::CHANNEL, packet);
        }
    }

    dim_tasks->second.erase(it);

    if(dim_tasks->second.empty()) {
        // There are no more requests
        // to generate a chunk for that
        // dimension, at least for now
        active_tasks.erase(dim_tasks);
    }
}

bool worldgen::is_generating(Dimension *dimension, const chunk_pos &cpos)
{
    auto dim_tasks = active_tasks.find(dimension);

    if(dim_tasks == active_tasks.cend()) {
        // No tasks for this dimension
        return false;
    }

    auto it = dim_tasks->second.find(cpos);

    if(it == dim_tasks->second.cend()) {
        // Not generating this chunk
        return false;
    }

    return true;
}

void worldgen::request_chunk(Session *session, const chunk_pos &cpos)
{
    if(session->dimension) {
        auto dim_tasks = active_tasks.find(session->dimension);

        if(dim_tasks == active_tasks.cend()) {
            dim_tasks = active_tasks.emplace(session->dimension, emhash8::HashMap<chunk_pos, std::unordered_set<Session *>>()).first;
        }

        auto it = dim_tasks->second.find(cpos);

        if(it == dim_tasks->second.cend()) {
            auto &sessions = dim_tasks->second.insert_or_assign(cpos, std::unordered_set<Session *>()).first->second;
            sessions.insert(session);

            threading::submit<WorldgenTask>(session->dimension, cpos);

            return;
        }

        it->second.insert(session);
    }
}
