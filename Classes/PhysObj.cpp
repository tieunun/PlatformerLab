#include "PhysObj.h"
/**
 * PhysObj.cc
 */
#include "Globals.h"

PhysObj::PhysObj()
    : _mass(10.f),
    _acceleration(Vec2(0,0)),
    _velocity(Vec2(0,0)),
    _aaBoundingBox(Rect(0,0,0,0))
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
}

bool PhysObj::isGrounded() { return !_airborn; }
bool PhysObj::isAirborn() { return _airborn; }

const Rect& PhysObj::getAABoundingBox() { return _aaBoundingBox; }
void PhysObj::setAABoundingBox(const Rect& bb) { _aaBoundingBox = bb; }

const Vec2& PhysObj::getAAOffset() { return _aaOffset; }
void PhysObj::setAAOffset(const Vec2& offset) { _aaOffset = offset; }


// Debug assistance
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
    Rect& aabb = this->_aaBoundingBox;
    Rect& bb = this->boundingBox();
    DrawPrimitives::drawSolidRect(
        aabb.origin + _aaOffset,
        Vec2(aabb.size.width * TILE_SIZE, aabb.size.height * TILE_SIZE)
        + _aaOffset,
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
    Vec2 position = this->getPosition() + _velocity;

    // Tile collision
    position = this->tileCollision(position);

    // Conclude Position
    this->setPosition(position);
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
        Vec2 rforce = force / _mass;
        _acceleration += rforce;
    }
}



const Vec2& PhysObj::tileCollision(const Vec2& position)
{
    Vec2 ret = Vec2(position); // predicted position
    Vec2 curPos = this->getPosition(); // Current position
    Vec2 prePos = position; // Predicted position
    Rect bb = this->boundingBox();

    Vec2 diff = prePos - curPos; // Difference in position (x and y)
    // in this case, since i'm subtracting current position from predicted
    // position, it should be relatively correct

    if(diff.x == 0 && diff.y == 0)
        return ret; // skip everything if no difference in movement.

    Rect curBB = Rect(
        _aaBoundingBox.origin.x + bb.origin.x + _aaOffset.x,
        _aaBoundingBox.origin.y + bb.origin.y + _aaOffset.y,
        _aaBoundingBox.size.width * TILE_SIZE,
        _aaBoundingBox.size.height * TILE_SIZE
        );
    Rect preBB = Rect(
        curBB.origin.x + diff.x,
        curBB.origin.y + diff.y,
        curBB.size.width,
        curBB.size.height
        );

    // step 1: get rect area of tiles.
    Vec2 start = Vec2(
        std::min(curBB.getMinX(), preBB.getMinX()) / TILE_SIZE,
        std::min(curBB.getMinY(), preBB.getMinY()) / TILE_SIZE
        );
    Vec2 finish = Vec2(
        std::max(curBB.getMaxX(), preBB.getMaxX()) / TILE_SIZE,
        std::max(curBB.getMaxY(), preBB.getMaxY()) / TILE_SIZE
        );

    
    // loop through each column.
    for(float y = start.y; y < finish.y; y++)
    {
        // loop through each row.
        for(float x = start.x; x < finish.x; x++)
        {
            Vec2 tilePos = tileCoord(Vec2(x,y));
            CCLOG("Testing tile pos [%f, %f]...", tilePos.x, tilePos.y);
            if(tilePos.y < 21 && tilePos.y > 20)
                CCLOG("gotcha");

            
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
                    //float tx = tilePos.x;
                    //ret = Vec2(
                    //    (tx + 1) * TILE_SIZE + ( _aaBoundingBox.size.width * TILE_SIZE ) + _aaOffset.x,
                    //    ret.y
                    //    );
                }
                if(diff.y > 0) // moved up
                {

                }
                if(diff.y < 0) // moved down
                {
                    // The below will not work.
                    // looks like i'll have to test intersections here.
                    float ty = std::ceil(_tileMap->getMapSize().height - tilePos.y);
                    ret = Vec2( // this won't work because the aabb size is different from the sprite.
                        ret.x,
                        (ty * TILE_SIZE) + ( _aaBoundingBox.size.height * TILE_SIZE)
                        );
                    _velocity.y = 0;
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

bool PhysObj::isCollidable(const Vec2& position)
{
    Vec2 corrected = Vec2(
        std::floor(position.x), std::floor(position.y));
    return isCollidable(_tileMap->getLayer("Meta")->getTileGIDAt(corrected));
}
    
/* Gets a tile based on literal position */
const Vec2& PhysObj::posToTileCoord(const Vec2& position)
{
    float tileSize = _tileMap->getTileSize().height; // width and height should be the same.
        
    int x = position.x / tileSize;
    int y = ((_tileMap->getMapSize().height * tileSize) - position.y)
        / tileSize;
    return Vec2(x, y);
}

// gets tile coordinate within map
const Vec2& PhysObj::tileCoord(const Vec2& position)
{
    Vec2 corrected = Vec2(
        std::floor(position.x),
        std::floor(_tileMap->getMapSize().height - position.y)
        );
    return Vec2(
        std::max(std::min(corrected.x, _tileMap->getMapSize().width - 0.1f), 0.f),
        std::max(std::min(corrected.y, _tileMap->getMapSize().height - 0.1f), 0.f)
        );
}

