#include "PhysObj.h"
/**
 * PhysObj.cc
 */
#include "Globals.h"

PhysObj::PhysObj()
    : _mass(10.f),
    _acceleration(Vec2(0,0)),
    _velocity(Vec2(0,0)),
    _collider(Rect(0,0,16,16)),
    _tileSize(16.f)
{ }
    
PhysObj* PhysObj::create(const std::string& filename)
{
    auto ret = new PhysObj();
    if(ret && ret->init(filename))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool PhysObj::init(const std::string& filename)
{
    if(!Sprite::initWithFile(filename))
        return false;

    this->getTexture()->setAliasTexParameters();

    return true;
}

// Properties
//-------------------------------------------------------------------------
float PhysObj::getMass() { return _mass; }
void PhysObj::setMass(float mass) { _mass = mass; }

TMXTiledMap* PhysObj::getTileMap() { return _tileMap; }
void PhysObj::setTileMap(TMXTiledMap* tilemap) 
{ 
    _tileMap = tilemap; 
    _metaLayer = tilemap->getLayer("Meta");
    _tileSize = tilemap->getTileSize().width;
}

bool PhysObj::isGrounded() { return !_airborn; }
bool PhysObj::isAirborn() { return _airborn; }

const Rect& PhysObj::getCollider() { return _collider; }
void PhysObj::setCollider(const Rect& bb) { _collider = bb; }

/* gets the collider's center point based on the sprite's anchor point */
const Vec2& PhysObj::getColliderPosition()
{
    return Vec2(
        boundingBox().origin.x + _collider.origin.x + (_collider.size.width * 0.5f),
        boundingBox().origin.y + _collider.origin.y + (_collider.size.height * 0.5f)
        );
}

/* sets the collider's position and the sprite as well */
void PhysObj::setColliderPosition(const Vec2& position)
{
    float bbWidth = boundingBox().size.width;
    float bbHeight = boundingBox().size.height;

    Vec2 bbOffset = Vec2(bbWidth * 0.5f, bbHeight * 0.5f);
    Vec2 cOffset = Vec2(_collider.size.width * 0.5f, _collider.size.height * 0.5f);
    
    Vec2 offset = Vec2(
        this->getPositionX() - (bbWidth * this->getAnchorPoint().x),
        this->getPositionY() - (bbHeight * this->getAnchorPoint().y)
        );
    this->setPosition(
        position.x + (bbOffset.x - _collider.origin.x - cOffset.x),
        position.y + (bbOffset.y - _collider.origin.y - cOffset.y)
        );
    //this->setPosition( // the below should work.
    //    position.x - (offset.x - (_collider.origin.x + _collider.size.height * 0.5f)),
    //    position.y - (offset.y - (_collider.origin.y + _collider.size.height * 0.5f))
    //    );
}

// Draw events (for debugging)
//-------------------------------------------------------------------------
void PhysObj::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
    Sprite::draw(renderer, transform, flags);

    _cmd.init(_globalZOrder);
    _cmd.func = CC_CALLBACK_0(PhysObj::onDraw, this, transform);

    renderer->addCommand(&_cmd);

}

void PhysObj::onDraw(const Mat4 &transform)
{
    auto director = Director::sharedDirector();
    director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);
        
    // Draw axis-aligned bounding box.
        
    Color4F color = Color4F(0,0, 0.5, 0.5);
    Rect& aabb = this->_collider;
    Rect& bb = this->boundingBox();
    DrawPrimitives::drawSolidRect(
        aabb.origin,
        Vec2(aabb.size.width, aabb.size.height)
        + aabb.origin,
        color
        );

    // Anchor Point
    Vec2 anc = this->getAnchorPoint();
    DrawPrimitives::drawCircle(
        Vec2(anc.x * bb.size.width, anc.y * bb.size.height), 
        2.f, 0.f, 4, false);


    //DrawPrimitives::drawRect(Vec2(0,0), bb.size);

    director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}





    
// Update
//-------------------------------------------------------------------------
void PhysObj::step()
{
    // Update velocity
    _velocity += _acceleration;

    float vx = _velocity.x;
    float vy = _velocity.y;

    // Correct velocity
    if(abs(vx) > MAX_VELOCITY) _velocity.x = (vx / abs(vx)) * MAX_VELOCITY;
    if(abs(vy) > MAX_VELOCITY) _velocity.y = (vy / abs(vy)) * MAX_VELOCITY;
    
    // Predicted Position
    Vec2 colliderPos = this->getColliderPosition();
    Vec2 position = Vec2(
        colliderPos.x + _velocity.x,
        colliderPos.y + _velocity.y
        );

    // Tile collision
    position = this->tileCollision(position);

    // Conclude Position
    this->setColliderPosition(position);
}
    
    
void PhysObj::update(float dt)
{
    // Gravity
    this->applyForce(Vec2(0, PHYS_GRAVITY * dt * -1));

    this->step();
}

// Methods
//-------------------------------------------------------------------------
void PhysObj::applyForce(Vec2 force)
{
    if(_mass > 0)
    {
        Vec2 rforce = Vec2(force.x / _mass, force.y / _mass);
        _acceleration += rforce;
    }
}



const Vec2& PhysObj::tileCollision(const Vec2& position)
{
    Vec2 ret = Vec2(position); // predicted position
    Vec2 curPos = this->getColliderPosition(); // Current position
    Vec2 prePos = Vec2(position); // Predicted position
    Rect bb = this->boundingBox();

    Vec2 diff = prePos - curPos; // Difference in position (x and y)
    // in this case, since i'm subtracting current position from predicted
    // position, it should be relatively correct

    if(diff.x == 0 && diff.y == 0)
        return ret; // skip everything if no difference in movement.

    Rect curBB = Rect( // current position bounding box
        _collider.origin.x + bb.origin.x + _collider.origin.x,
        _collider.origin.y + bb.origin.y + _collider.origin.y,
        _collider.size.width,
        _collider.size.height
        );
    Rect preBB = Rect( // predicted position bounding box
        curBB.origin.x + diff.x,
        curBB.origin.y + diff.y,
        curBB.size.width,
        curBB.size.height
        );

    // bottom left
    float x0 = std::min(curBB.getMinX(), preBB.getMinX());
    float y0 = std::min(curBB.getMinY(), preBB.getMinY());

    // top right
    float x1 = std::max(curBB.getMaxX(), preBB.getMaxX());
    float y1 = std::max(curBB.getMaxY(), preBB.getMaxY());

    // step 1: get rect area of tiles.
    Vec2 start = toTileCoord(Vec2(x0,y0));
    Vec2 finish = toTileCoord(Vec2(x1,y1));

    // loop through each column.
    for(float y = start.y; y >= finish.y; y--)
    {
        // loop through each row.
        for(float x = start.x; x <= finish.x; x++)
        {
            Vec2 tilePos = Vec2(x,y);
            CCLOG("Testing tile pos [%f, %f]...", tilePos.x, tilePos.y);

            if( isCollidable( tilePos ) )
            {
                
                // directional tests
                if(diff.x > 0) // moved right
                {
                    
                    //float tx = tilePos.x;
                    //ret = Vec2(
                    //    tx * TILE_SIZE + ( _aaBoundingBox.size.width * TILE_SIZE ) + _aaOffset.x,
                    //    ret.y
                    //    );
                }
                if(diff.x < 0) // moved left
                {
                    auto tile = _metaLayer->getTileAt(tilePos);

                    // TODO: Collision
                    //if(preBB.intersectsRect(tile->getBoundingBox()))
                    //{
                    //    
                    //    ret = Vec2(
                    //        tile->getBoundingBox().getMinX()
                    //        + _collider.size.width * 0.5f,
                    //        ret.y
                    //        );
                    //}
                }
                if(diff.y > 0) // moved up
                {

                }
                if(diff.y < 0) // moved down
                {
                    // As suggested, i need to test the topmost intersection
                    // the AA bounding box has with the tile.
                    auto tile = _metaLayer->getTileAt(tilePos);
/*
                    float tileY = tile->getBoundingBox().getMaxY();
                    float minY = preBB.origin.y;

                    if(minY < tileY)
*/
                    // TODO: Collision
                    //if(preBB.intersectsRect(tile->getBoundingBox()))
                    //{
                    //    
                    //    ret = Vec2(
                    //        ret.x,
                    //        tile->getBoundingBox().getMaxY()
                    //        + _collider.size.height * 0.5f
                    //        );
                    //    _airborn = false; // grounded
                    //    _velocity = Vec2(_velocity.x, 0);
                    //}
                }
            }
        }
    }

    return ret;
}


/* Determines of a tile (gid) is collidable or not. */
bool PhysObj::isCollidable(uint32_t gid)
{
    if(!gid) return false;

    auto properties = _tileMap->getPropertiesForGID(gid);
    if(properties.isNull()) 
        return false;

    auto propMap = properties.asValueMap();
    auto value = propMap.at("isCollidable");
            
    if(value.asString() == "true") {
        CCLOG("  Tile is collidable");
        return true;
    }

    return false;
}

/* Determines of a tile (position) is collidable or not. */
bool PhysObj::isCollidable(const Vec2& coord)
{
    //Vec2 corrected = Vec2(std::floor(position.x), std::floor(position.y));
    return isCollidable(_metaLayer->getTileGIDAt(coord));
}

// gets tile coordinate within map
const Vec2& PhysObj::mapCoord(const Vec2& coord)
{
    Vec2 corrected = Vec2(coord.x,
        _tileMap->getMapSize().height - coord.y
        );
    return Vec2( // the -0.01f is to ensure it stays within the map.
        std::max(std::min(corrected.x, _tileMap->getMapSize().width), 0.f),
        std::max(std::min(corrected.y, _tileMap->getMapSize().height), 0.f)
        );
    
}

// converts a point to a tile coordinate.
const Vec2& PhysObj::toTileCoord(const Vec2& point)
{
    int x = point.x / _tileSize;
    int y = ((_tileMap->getMapSize().height * _tileSize) - point.y) / _tileSize;

    return Vec2(x, y);
}

void PhysObj::moveLeft() { _velocity.x = -2.f; }
void PhysObj::moveRight() { _velocity.x = 2.f; }
void PhysObj::stop() { _velocity.x = 0.f; }

void PhysObj::setVelocity(const Vec2& velocity) { _velocity = velocity; }