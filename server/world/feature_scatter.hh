#ifndef F63690CB_F2D8_41C3_88F8_CCA3EBDBDC80
#define F63690CB_F2D8_41C3_88F8_CCA3EBDBDC80

#include "server/world/feature_placer.hh"

class FeatureScatter final : public PlacementRule {
public:
    static void init(void);

    virtual ~FeatureScatter(void) override = default;
    virtual void collect(const ChunkPos& pos, const PlacementContext& context, std::vector<FeatureInstance>& out) const override;
};

#endif /* F63690CB_F2D8_41C3_88F8_CCA3EBDBDC80 */
