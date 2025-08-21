//感谢 EasyX 图形库提供的便捷绘图接口，以及各类开源资源网站提供的游戏素材。欢迎提交 issues 或 PR 帮助改进这个项目！ 
#include<iostream>
#include<graphics.h>
#include<vector>
#include<easyx.h>
#include<ctime>
#include<windows.h>
#pragma comment(lib, "EasyXa.lib")  // 确保链接正确库
#include<map>
#include<string>
#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")
using namespace std;
//这里constexpr 是常量表达式，这么写能在编译是确定值
constexpr auto width = 500;
constexpr auto height = 700;

//判断点击是否有效
bool judgepoint(int x, int y, const RECT& r)
{
	return (r.left <= x && x <= r.right && r.top <= y && y <= r.bottom);
}

//定义背景类
class BK
{
	IMAGE& img;
	int y;
public:
	//拷贝
	BK(IMAGE& img) :img(img), y(-height) {}

	void Show()
	{
		if (y == 0)
		{
			y = -height;
		}
		y += 4;//实现滚动
		putimage(0, y, &img);//绘制图，x,y,指针

	}
};

//定义自己的飞机类
class Myplane
{
	IMAGE& img;
	RECT t;
	int shootcool;
	int shootgap;
public:
	Myplane(IMAGE& img) :img(img),shootcool(0),shootgap(5)
	{
		t.left = width / 2 - img.getwidth() / 2;
		t.top = height - img.getheight();
		t.right = width / 2 + img.getwidth() / 2;
		t.bottom = height;
	}

	//能否发射子弹
	bool canShoot()const
	{
		return shootcool <= 0;
	}

	void updatecool()
	{
		if (shootcool > 0)
		{
			shootcool--;
		}
	}

	void shoot()
	{
		//每发一枚，重载等待时间
		shootcool = shootgap;
	}

	//获取子弹发射的位置   POINT是结构体
	POINT getbulletxy()const
	{
		POINT pos;
		pos.x = (t.left + t.right) / 2;
		pos.y = t.top;
		return pos;
	}

	void Show()
	{
		putimage(t.left, t.top, &img);
	}

	void Control()
	{
		//消息结构体
		ExMessage mess;
		//鼠标位置
		if (peekmessage(&mess, EM_MOUSE))
		{
			t.left = mess.x - img.getwidth() / 2;
			t.top = mess.y - img.getheight() / 2;
			t.right = t.left + img.getwidth();
			t.bottom = t.top + img.getheight();

		}
	}

	RECT getRect()const
	{
		return t;
	}
};

//添加敌机类型
enum class EnemyType
{
	TYPE1,TYPE2
};

//定义敌机类
class Enemy
{
	IMAGE& img;
	RECT t;
	int speed;
	EnemyType type;
	int score;
public:
	Enemy(IMAGE& enemyimg1,IMAGE& enemyimg2 ,int x,EnemyType enemytype) 
		:type(enemytype), img(enemytype == EnemyType::TYPE1 ? enemyimg1 : enemyimg2)
	{

		//类型1
		if (type == EnemyType::TYPE1)
		{
			img = enemyimg1;
			speed = 4 + rand() % 3;//4-6
			score = 20;
		}
		else
		{

			img = enemyimg2;
			speed = 2 + rand() % 2;//2-3
			score = 10;
		
		}

		t.left = x;
		t.top = -img.getheight();
		t.right = t.left + img.getwidth();
		t.bottom = t.top + img.getheight();
		speed = 3 + rand() % 3;//速度在3-5之间
	}

	//获取敌机类型
	EnemyType getType()const
	{
		return type;
	}

	//获取分数
	int getScore()const
	{
		return score;
	}
	bool Show()
	{
		if (t.top >= height)
		{
			return false;
		}

		t.top += speed;
		t.bottom += speed;
		putimage(t.left, t.top, &img);
		return true;
	}
	RECT getRect() const 
	{
		return t;
	}
};

//定义子弹类
class Bullet
{
	IMAGE& img;
	RECT t;
	int speed;
	bool active;
public:
	Bullet(IMAGE& bulletimg, int x, int y) :img(bulletimg),
		speed(10), active(true)
	{
		t.left = x - img.getwidth() / 2;
		t.top = y - img.getheight();
		t.right = t.left + img.getwidth();
		t.bottom = t.top + img.getheight();
	}

	bool Show()
	{
		if (!active || t.bottom <= 0)
		{
			return false;
		}
		
		t.top -= speed;
		t.bottom -= speed;
		putimage(t.left, t.top, &img);
		return true;
	}

	void setActive(bool isActive)
	{
		active = isActive;
	}

	bool isActive()const
	{
		return active;
	}

	RECT getRect()const
	{
		return t;
	}

	//检查子弹是否击中敌机
	bool checkhit(const Enemy* enemy)const
	{
		//???
		if (!active || !enemy)
		{
			return false;
		}
		
		RECT bulletRect = getRect();
		RECT enemyRect = enemy->getRect();

		//???
		return !(bulletRect.right<enemyRect.left ||
			bulletRect.left>enemyRect.right ||
			bulletRect.top > enemyRect.bottom ||
			bulletRect.bottom < enemyRect.top);
	}
};

//定义音效
class Audio
{
	map<string, string>sounds;

	Audio() {}
	//显示删除拷贝构造函数
	Audio(const Audio&) = delete;
	//显示删除拷贝赋值运算符
	Audio& operator=(const Audio&) = delete;

public:
	//确保整个程序中，Audio 类只有一个实例，并提供一个全局访问点来获取这个唯一的实例、
	//只会在第一次调用 getInstance() 时执行，创建这个唯一的 Audio 对象。之后的所有调用都会跳过创建
	static Audio& getInstance()
	{
		static Audio instance;
		return instance;
	}

	//加载音效
	bool loadAudio(const string& alias, const string& filepath)
	{
		//如果找到了别名
		if (sounds.find(alias) != sounds.end())
		{
			char closeCmd[300];
			sprintf_s(closeCmd, "close %s", alias.c_str());//拼接命令字符串
			mciSendStringA(closeCmd, NULL, 0, NULL);//执行命令
		}
		
		char openCmd[600];
		sprintf_s(openCmd, "open \"%s\" alias %s", filepath.c_str(), alias.c_str());
		//执行打开命令，并检查是否成功
		if (mciSendStringA(openCmd, NULL, 0, NULL) != 0)
		{
			return false;
		}

		sounds[alias] = filepath;
		return true;
	}

	//播放音效                     
	bool playSound(const string& alias, bool loop = false)
	{
		//没找到别名
		if (sounds.find(alias) == sounds.end())
		{
			return false;
		}

		char playCmd[300];
		if (loop)
		{
			//循环
			sprintf_s(playCmd, "play %s repeat", alias.c_str());
		}
		else
		{
			//一次
			sprintf_s(playCmd, "play %s", alias.c_str());

		}
		return mciSendStringA(playCmd, NULL, 0, NULL) == 0;
	}

	//停止音效
	bool stopAudio(const string& alias)
	{
		if (sounds.find(alias) == sounds.end())
		{
			return false;
		}

		char stopCmd[300];
		sprintf_s(stopCmd, "stop %s", alias.c_str());
		return mciSendStringA(stopCmd, NULL, 0, NULL) == 0;
	}

	//设置音量
	bool setVolume(const string& alias, int volume)
	{
		if (sounds.find(alias) == sounds.end() || volume < 0 || volume>100)
		{
			return false;
		}

		char volumeCmd[300];
		int mciVolume = volume * 10;
		sprintf_s(volumeCmd, "setaudio %s volume to %d", alias.c_str(),mciVolume);
		return mciSendStringA(volumeCmd, NULL, 0, NULL) == 0;

	}

	//关闭
	void closeAll()
	{
		for (const auto& sound : sounds)
		{
			char closeCmd[300];
			sprintf_s(closeCmd, "close %s", sound.first.c_str());
			mciSendStringA(closeCmd, NULL, 0, NULL);
		}
		sounds.clear();
	}

	~Audio()
	{
		closeAll();
	}
};

//检查新敌机与现存的有无重叠
bool isoverlap(const Enemy* newEnemy, const vector<Enemy*>& enemies,int padding=5)
{
	//获取新的矩形区域
	RECT newrect = newEnemy->getRect();
	newrect.left -= padding;
	newrect.right += padding;
	newrect.top -= padding;
	newrect.bottom += padding;

	//遍历所以现存的敌机
	for (const auto* enemy : enemies)
	{
		if (!enemy)
		{
			continue;
		}
		//AABB碰撞检测算法
		RECT existrect = enemy->getRect();
		if (!(newrect.right<existrect.left ||
			newrect.left>existrect.right ||
			newrect.bottom<existrect.top ||
			newrect.top>existrect.bottom))
		{
			return true;//重叠
		}
	}
	return false;
}

//生成敌机+检测
void spawnNewEnemies(vector<Enemy*>& enemies, IMAGE& enemyimg1,IMAGE& enemyimg2, int k)
{
	while (enemies.size() < k)
	{
		//新建敌机 0.7->type1 0.3->type2
		EnemyType type = (rand() % 10 < 7) ? EnemyType::TYPE1 : EnemyType::TYPE2;

		//根据类型获取宽度
		int enemy_w = (type == EnemyType::TYPE1) ? enemyimg1.getwidth() : enemyimg2.getwidth();
		int x = rand() % (width - enemy_w);
		Enemy* newEnemy = new Enemy(enemyimg1, enemyimg2, x, type);

		//检查有无重叠
		if (!isoverlap(newEnemy, enemies,5))
		{
			enemies.push_back(newEnemy);
		}
		else
		{
			delete newEnemy;
			// 防止无限循环的安全机制
			static int retryCount = 0;
			retryCount++;
			if (retryCount > 100)
			{
				// 多次尝试失败后强制添加，避免死锁
				enemies.push_back(new Enemy(enemyimg1,enemyimg2, x,type));
				retryCount = 0;
				break;
			}
		}
	}
}

//开始界面
void welcome()
{
	//Windows API中表示"指向常量TCHAR字符串的长指针",_T()自适应字符集
	LPCTSTR title = _T("飞机大战");
	LPCTSTR startplay = _T("开始游戏");
	LPCTSTR endplay = _T("结束游戏");

	BeginBatchDraw();//开启绘画
	setbkcolor(WHITE);//设置背景白色
	cleardevice();

	//输出”飞机大战“
	//参数：字体高度 宽度（0为自适应） 指定字体
	settextstyle(60, 0, _T("黑体"));
	settextcolor(BLACK);
	outtextxy(width / 2 - textwidth(title) / 2, height / 8, title);

	//定义两个矩形框
	//RECT 是 Windows API 中定义的一个矩形结构体，包括left top right bottom
	RECT startplayr, endplayr;
	settextstyle(40, 0, _T("黑体"));

	// 开始游戏按钮区域
	startplayr.left = width / 2 - textwidth(startplay) / 2;
	startplayr.top = height / 5*2;
	startplayr.right = startplayr.left + textwidth(startplay);
	startplayr.bottom = startplayr.top + textheight(startplay);  

	// 结束游戏按钮区域
	endplayr.left = width / 2 - textwidth(endplay) / 2;
	endplayr.top = height / 2; 
	endplayr.right = endplayr.left + textwidth(endplay);
	endplayr.bottom = endplayr.top + textheight(endplay);

	// 1. 绘制开始游戏按钮（红色填充+黑色边框）
	setfillcolor(RED);
	fillrectangle(startplayr.left, startplayr.top, startplayr.right, startplayr.bottom);
	setlinecolor(BLACK);
	rectangle(startplayr.left, startplayr.top, startplayr.right, startplayr.bottom);

	// 2. 绘制结束游戏按钮（蓝色填充+黑色边框）
	setfillcolor(BLUE);
	fillrectangle(endplayr.left, endplayr.top, endplayr.right, endplayr.bottom);
	setlinecolor(BLACK);
	rectangle(endplayr.left, endplayr.top, endplayr.right, endplayr.bottom);

	// 3. 在按钮上绘制文字（白色文字+透明背景）
	settextcolor(WHITE);
	setbkmode(TRANSPARENT);//设置当前设备图案填充和文字输出时的背景模式(透明模式)
	outtextxy(startplayr.left, startplayr.top, startplay);
	outtextxy(endplayr.left, endplayr.top, endplay);

	EndBatchDraw();

	while (1)
	{
		//申明消息结构体变量，包括鼠标坐标，安下左还是右建
		ExMessage mess;
		//如果获取到了消息
		if (peekmessage(&mess, EM_MOUSE))
		{
			//鼠标左键
			if (mess.lbutton)
			{
				//判断点击是否有效
				if (judgepoint(mess.x, mess.y, startplayr))
				{
					//清空消息缓冲区（鼠标消息）
					flushmessage(EM_MOUSE);
					return;
				}
				else if (judgepoint(mess.x, mess.y, endplayr))
				{
					exit(0);
				}
			}
		}
		Sleep(10);
	}
}

//检测玩家与敌机碰撞
bool checkcollision(const Myplane& plane, const vector<Enemy*>& enemies)
{
	RECT planerect = plane.getRect();

	for (const auto* enemy : enemies)
	{
		if (!enemy)
		{
			continue;
		}
		//AABB碰撞检测算法
		RECT enemyrect = enemy->getRect();

		if (!(planerect.right<enemyrect.left ||
			planerect.left>enemyrect.right ||
			planerect.bottom<enemyrect.top ||
			planerect.top>enemyrect.bottom))
		{
			return true;//碰撞
		}
	}
	return false;
}

//游戏结束画面
void gameover(int score = 0)
{
	Audio::getInstance().stopAudio("background");
	Audio::getInstance().playSound("gameover");
	BeginBatchDraw();

	//绘制半透明遮罩，前三个红绿蓝=0==黑， 透明度
	setfillcolor(RGB(0, 0, 0, 180));//半透明黑色
	solidrectangle(0, 0, width, height);//绘制实心矩形填充颜色，左上右下点

	//绘制游戏结束字
	settextstyle(80, 0, _T("黑体"));
	settextcolor(RED);
	LPCTSTR gameover_text = _T("游戏结束");
	outtextxy(width / 2 - textwidth(gameover_text) / 2, height / 3, gameover_text);

	//绘制当局分数
	TCHAR scoretext[50];//自适应字符类型
	_stprintf_s(scoretext, _T("分数：%d"), score);
	settextstyle(50, 0, _T("黑体"));
	settextcolor(YELLOW);
	outtextxy(width / 2 - textwidth(scoretext) / 2, height / 2, scoretext);

	//绘制提示
	settextstyle(30, 0, _T("黑体"));
	settextcolor(WHITE);
	LPCTSTR reminder = _T("点击任意位置返回开始界面");
	outtextxy(width / 2 - textwidth(reminder) / 2, height / 3 * 2, reminder);

	EndBatchDraw();

	//等待点击
	ExMessage msg;
	while (1)
	{
		if (peekmessage(&msg))
		{
			//左键按下
			if (msg.message == WM_LBUTTONDOWN)
			{
				break;
			}
			//键盘ESC
			if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE)
			{
				exit(0);
			}
		}
		Sleep(10);
	}
}

//开始游戏后
void Play()
{
	//初始化音效  不加& 会引起拷贝，类明确禁止拷贝
	Audio& sound = Audio::getInstance();

	//加载音效
	sound.loadAudio("background", "your filepath");
	sound.loadAudio("bullet", "your filepath");
	sound.loadAudio("explosion", "ypur filepath");
	sound.loadAudio("gameover", "your filepath");

	//设置音量
	sound.setVolume("background", 30);
	sound.setVolume("bullet", 60);
	sound.setVolume("explosion", 70);
	sound.setVolume("gameover", 80);

	//循环背景音乐
	sound.playSound("background", true);

	setbkcolor(WHITE);
	cleardevice();

	IMAGE mineimg, bkimg, enemyimg1,enemyimg2, bulimg;
	// 1. 调整我的飞机图片大小（第二个和第三个参数指定宽高）
	loadimage(&mineimg, _T("D:\\mineimg2.jpg"), 60, 60);  // 调整为60x60大小，可根据需要修改
	loadimage(&bkimg, _T("D:\\bkimg2.jpg"), width, 2 * height);
	loadimage(&enemyimg1, _T("D:\\enemyimg2.jpg"), 30, 30);
	loadimage(&enemyimg2, _T("D:\\enemyimg3.jpg"), 20, 20);
	loadimage(&bulimg, _T("D:\\bulimg2.jpg"), 10, 20);

	BK bk = BK(bkimg);
	Myplane air = Myplane(mineimg);

	vector<Enemy*>es;
	vector<Bullet*>bullets;

	// 3. 确保敌机初始位置正确
	spawnNewEnemies(es, enemyimg1,enemyimg2, 4);

	//分数
	int score = 0;
	bool running = true;

	//自动射击计数器
	int autoshootTime = 0;
	const int autoshootgap = 8;

	while (running)
	{
		BeginBatchDraw();
		cleardevice();

		bk.Show();
		//更新射击冷却
		air.updatecool();

		autoshootTime++;
		if (autoshootTime >= autoshootgap && air.canShoot())
		{
			POINT bulletpos = air.getbulletxy();
			bullets.push_back(new Bullet(bulimg, bulletpos.x, bulletpos.y));
			air.shoot();
			autoshootTime = 0;

			sound.playSound("bullet");
		}

		//鼠标点击射击
		ExMessage msg;
		if (peekmessage(&msg, EM_MOUSE) && msg.message == WM_LBUTTONDOWN && air.canShoot())
		{
			POINT bulletPos = air.getbulletxy();
			bullets.push_back(new Bullet(bulimg, bulletPos.x, bulletPos.y));
			air.shoot();

			sound.playSound("bullet");
		}

		air.Control();
		air.Show();

		//显示和更新子弹
		auto bulletit = bullets.begin();
		while (bulletit != bullets.end())
		{
			if (!(*bulletit)->Show())
			{
				delete (*bulletit);
				bulletit = bullets.erase(bulletit);
			}
			else
			{
				bulletit++;
			}
		}

		//显示和更新敌机
		auto enemyit = es.begin();
		while (enemyit != es.end())
		{
			if (!(*enemyit)->Show())
			{
				delete (*enemyit);
				enemyit = es.erase(enemyit);
			}
			else
			{
				bool hit = false;
				for (auto bullet : bullets)
				{
					if (bullet && bullet->isActive() && bullet->checkhit(*enemyit))
					{
						bullet->setActive(false);
						hit = true;
						score += (*enemyit)->getScore();

						sound.playSound("explosion");
						break;
					}

				}

				if (hit)
				{
					delete(*enemyit);
					enemyit = es.erase(enemyit);
				}
				else
				{
					enemyit++;
				}
			}
		}

		// 控制敌机数量
		spawnNewEnemies(es, enemyimg1,enemyimg2, 5); // 最多5架敌机

		//显示当前分数
		TCHAR scoretext[50];
		_stprintf_s(scoretext, _T("分数：%d"), score);
		settextstyle(20, 0, _T("宋体"));
		settextcolor(BLACK);
		setbkmode(TRANSPARENT);
		outtextxy(10, 10, scoretext);
		//检测碰撞
		if (checkcollision(air, es))
		{
			running = false;
		}

		EndBatchDraw();
		Sleep(20);
	}

	// 释放所有敌机资源
	for (auto* enemy : es)
	{
		delete enemy;
	}
	for (auto* bullet : bullets)
	{
		delete bullet;
	}
	es.clear();
	bullets.clear();

	// 显示游戏结束界面
	gameover(score);
}

int main()
{
	//用于初始化图 参数：宽 高 禁止最小化和显示控制台窗口
	initgraph(width, height, EW_NOMINIMIZE | EW_SHOWCONSOLE);
	srand((unsigned int)time(NULL)); // 初始化随机数种子
	while (1)
	{
		welcome();
		Play();
	}
	
	//关音乐
	Audio::getInstance().closeAll();

	closegraph();
	return 0;
}
