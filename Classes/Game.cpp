#include "Game.h"
/**
 * Game.ccp
 *
 * @author Jonathan H (sarseo)
 */
#include "Globals.h"

Game::Game()
    : _jump(false)
{

}

Scene* Game::create()
{
    auto ret = new Game();
    if(ret && ret->init())
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Game::init()
{
    // check if parent scene isn't going to have a stroke.
    if(!Scene::init()) return false;

    // set up a nice clean background layer
    auto bg = LayerColor::create(ccc4(25,128,255,255));
    background = bg;
    this->addChild(background, -1);

    // set up main gaming layer.
    gameLayer = Layer::create();
    gameLayer->setAnchorPoint(Vec2(0,0));
    gameLayer->setScale(PXSCALE);
    this->addChild(gameLayer);

    // get some sweet tilemap sauce
    map = TMXTiledMap::create("map.tmx");
    map->getLayer("Meta")->setVisible(false);
    gameLayer->addChild(map);

    // getting the player sprite set up.
    player = PhysObj::create("sprite.png");
    player->setPosition(60, 75);

    player->setCollider(Rect(8,0,16,16));

    player->setTileMap(map);

    player->setVelocity(Vec2(-1.f,0.f));
    gameLayer->addChild(player);

    // Keyboard bindings
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = CC_CALLBACK_2(Game::onKeyPressed, this);
    listener->onKeyReleased = CC_CALLBACK_2(Game::onKeyReleased, this);

    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    this->scheduleUpdate();

    return true;
}

void Game::update(float dt)
{
    player->update(dt);
}

void Game::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
    //if(!_jump && keyCode == EventKeyboard::KeyCode::KEY_UP_ARROW) 
    //{
    //    _jump = true;
    //    player->applyForce(Vec2(0, 20.f));
    //}

    if(keyCode == EventKeyboard::KeyCode::KEY_LEFT_ARROW)
    {
        player->moveLeft();
    }
    
    if(keyCode == EventKeyboard::KeyCode::KEY_RIGHT_ARROW)
    {
        player->moveRight();
    }
}

void Game::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
    player->stop();
}