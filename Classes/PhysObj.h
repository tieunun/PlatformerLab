#ifndef _PLATFORMERLAB_PHYSOBJ_H_
#define _PLATFORMERLAB_PHYSOBJ_H_
/**
 * PhysObj.h
 *
 * This is a self-contained physics object designed for platformer tile-based
 * physics.
 *
 * Notes:
 *
 * - The *HitBox* of the PhysObj is actually the collidable area measured in
 * points.  By default, the hitbox is 16x16 in size and starts from the bottom
 * left of the sprite's bounding box.  The /origin/ attribute would represent
 * the hitbox's offset.
 */ 
#include "cocos2d.h"

USING_NS_CC;

class PhysObj : public Sprite
{
private:
    // Members
    //-------------------------------------------------------------------------
    Vec2 _velocity;         // The speed the object moves.
    Vec2 _acceleration;     // The accelleration on the object.
    float _mass;            // The mass of the object.

    TMXTiledMap* _tileMap;  // The tilemap that the object adheres to.
    TMXLayer* _metaLayer;   // The relevant meta layer of the tilemap.
    float _tileSize;        // The size of tiles in the tilemap.

    Rect _collider;           // The physics object's hit box.
    
    bool _airborn;

    // Private methods
    const Vec2& tileCollision(const Vec2& position);
    //const Vec2& posToTileCoord(const Vec2& position);
    bool isCollidable(uint32_t gid);
    bool isCollidable(const Vec2& coord);
    
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

    void setVelocity(const Vec2& velocity);

    TMXTiledMap* getTileMap();
    void setTileMap(TMXTiledMap* tilemap);

    const Rect& getCollider();
    void setCollider(const Rect& bb);

    const Vec2& getColliderPosition();
    void setColliderPosition(const Vec2& position);

    bool isGrounded();  // object is grounded.
    bool isAirborn();   // object is airborn.

    // This "steps" the physics object.
    void step();        

    // This is an update that applies basic forces and stuff.
    void update(float dt);

    void applyForce(Vec2 force);

    const Vec2& mapCoord(const Vec2& coord);

    const Vec2& toTileCoord(const Vec2& point);

    void moveLeft();
    void moveRight();
    void stop();
    void jump();
};


#endif /* _PLATFORMERLAB_PHYSOBJ_H_ */