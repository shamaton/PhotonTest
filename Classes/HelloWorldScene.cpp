#include "HelloWorldScene.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
	cocos2d::Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label
    
    auto label = LabelTTF::create("Hello World", "Arial", 24);
    
    // position the label on the center of the screen
    label->setPosition(Vec2(origin.x + visibleSize.width/2,
                            origin.y + visibleSize.height - label->getContentSize().height));

    // add the label as a child to this layer
    this->addChild(label, 1);

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");

    // position the sprite on the center of the screen
    sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

    // add the sprite as a child to this layer
    this->addChild(sprite, 0);
	
	//
	
	// シングルタップリスナーを用意する
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(_swallowsTouches);
	
	// 各イベントの割り当て
	listener->onTouchBegan     = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
	listener->onTouchMoved     = CC_CALLBACK_2(HelloWorld::onTouchMoved, this);
	listener->onTouchEnded     = CC_CALLBACK_2(HelloWorld::onTouchEnded, this);
	listener->onTouchCancelled = CC_CALLBACK_2(HelloWorld::onTouchCancelled, this);
	
	// イベントディスパッチャにシングルタップ用リスナーを追加する
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	
	// Photonネットワーククラスのインスタンスを作成
	networkLogic = new NetworkLogic();
	//networkLogic = new NetworkLogic(L"1.0");
 
	// 毎フレームでupdateを実行させる
	this->schedule(schedule_selector(HelloWorld::update));
	this->scheduleUpdate();
    
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

bool HelloWorld::onTouchBegan(cocos2d::Touch* touch, cocos2d::Event* event)
{

	if (networkLogic->playerNr) {
		this->addParticle(networkLogic->playerNr, touch->getLocation().x, touch->getLocation().y);
		
		// イベント（タッチ座標）を送信
		ExitGames::Common::Hashtable* eventContent = new ExitGames::Common::Hashtable();
		eventContent->put<int, float>(1, touch->getLocation().x);
		eventContent->put<int, float>(2, touch->getLocation().y);
		networkLogic->sendEvent(1, eventContent);
	}
 
	return true;
}

void HelloWorld::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *unused_event) {
	
}

void HelloWorld::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *unused_event) {
	
}

void HelloWorld::onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *unused_event){
	
}

void HelloWorld::update(float delta)
{
	networkLogic->run();
 
	switch (networkLogic->getState()) {
		case STATE_CONNECTED:
			CCLOG("connected");
		case STATE_LEFT:
			// ルームが存在すればジョイン、なければ作成する
			if (networkLogic->isRoomExists()) {
				CCLOG("Join");
				networkLogic->setLastInput(INPUT_2);
			} else {
				CCLOG("Create");
				networkLogic->setLastInput(INPUT_1);
			}
			break;
		case STATE_DISCONNECTED:
			CCLOG("disconnected");
			// 接続が切れたら再度接続
			networkLogic->connect();
			break;
		case STATE_CONNECTING:
		case STATE_JOINING:
		case STATE_JOINED:
		case STATE_LEAVING:
		case STATE_DISCONNECTING:
			//CCLOG("bbbbb");
		default:
			//CCLOG("default");
			break;
	}
 
	while (!networkLogic->eventQueue.empty()) {
		std::array<float, 3> arr = networkLogic->eventQueue.front();
		networkLogic->eventQueue.pop();
		
		int playerNr = static_cast<int>(arr[0]);
		float x = arr[1];
		float y = arr[2];
		CCLOG("%d, %f, %f", playerNr, x, y);
		
		this->addParticle(playerNr, x, y);
	}
}

void HelloWorld::addParticle(int playerNr, float x, float y)
{
	ParticleSystem* particle;
	switch (playerNr) {
			case 1:
			particle = ParticleFire::create();
			break;
			case 2:
			particle = ParticleSmoke::create();
			break;
			case 3:
			particle = ParticleFlower::create();
			break;
			default:
			particle = ParticleSun::create();
			break;
		}
	particle->setDuration(0.1);
	particle->setSpeed(500);
	particle->setPosition(cocos2d::Point(x,y));
	this->addChild(particle);
}
