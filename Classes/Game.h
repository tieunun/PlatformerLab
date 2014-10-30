#ifndef _PLATFORMERLAB_GAME_H_
#define _PLATFORMERLAB_GAME_H_
/**
 * Game.h
 *
 * @author Jonathan H (sarseo)
 */
#include "cocos2d.h"
#include "PhysObj.h"

USING_NS_CC;

class Game : public Scene
{
private:
    Layer* gameLayer;
    Layer* background;
    TMXTiledMap* map;

    bool _jump;

    PhysObj* player;
public:
    Game();

    static Scene* create();
    virtual bool init();

    virtual void update(float dt);

    void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
    void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
};

#endif /* _PLATFORMERLAB_GAME_H_ */