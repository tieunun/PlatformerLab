#ifndef _PLATFORMERLAB_PHYSOBJ_H_
#define _PLATFORMERLAB_PHYSOBJ_H_
/**
 * PhysObj.h
 */ 
#include "cocos2d.h"

// Tags

USING_NS_CC;

class PhysObj : public Sprite
{
private:
    // Members
    Vec2 _velocity;
    Vec2 _acceleration;
    float _mass;

    TMXTiledMap* _tileMap;
    TMXLayer* _metaLayer;
    Rect _aaBoundingBox;
    
    Vec2 _aaOffset;

    bool _airborn;

    // Private methods
    const Vec2& tileCollision(const Vec2& position);
    const Vec2& posToTileCoord(const Vec2& position);
    bool isCollidable(uint32_t gid);
    bool isCollidable(const Vec2& position);
    
    void onDraw(const Mat4 &transform);
    CustomCommand _cmd;
    

public:
    PhysObj();
    
    virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;

    static PhysObj* create(const std::string& spriteFrameName);
    virtual bool init(const std::string& spriteFrameName);

    // Properties
    float getMass();
    void setMass(float mass);

    TMXTiledMap* getTileMap();
    void setTileMap(TMXTiledMap* tilemap);

    const Rect& getAABoundingBox();
    void setAABoundingBox(const Rect& bb);

    const Vec2& getAAOffset();
    void setAAOffset(const Vec2& offset);

    bool isGrounded();  // object is grounded.
    bool isAirborn();   // object is airborn.

    // This "steps" the physics object.
    void step();        

    // This is an update that applies basic forces and stuff.
    void update(float dt);

    void applyForce(Vec2 force);

    const Vec2& tileCoord(const Vec2& position);
};


#endif /* _PLATFORMERLAB_PHYSOBJ_H_ */