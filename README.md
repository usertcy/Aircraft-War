# Aircraft-War
A simple aircraft war game based on C++.
这是一款竖版射击类游戏，玩家通过鼠标控制飞机移动，发射子弹击落敌机获得分数。游戏包含两种不同类型的敌机，每种敌机有不同的速度和得分。随着游戏进行，你需要躲避敌机并尽可能击落更多目标，挑战更高分数！

本项目是基于https://www.bilibili.com/video/BV1xY4y1k7hH/?p=5&spm_id_from=333.337.top_right_bar_window_history.content.click&vd_source=ceb9dd05a7b9aa6b97daf32a5be61bc4的框架完成，在原来基础上做了以下改进：
-”开始游戏”，“结束游戏”，做了矩形框并进行底色填充+描边框
-增加了敌机的类型，生成两种敌机
-游戏进行实时更新分数，游戏结束界面显示分数和相应提示
-增添背景音乐和音效（发射、爆炸、游戏结束）

代码结构
airplane-war/
├── main.cpp               # 主程序入口
├── resources/             # 游戏资源（图片、音效）
│   ├── mineimg2.jpg       # 玩家飞机图片
│   ├── enemyimg2.jpg      # 敌机1图片
│   ├── enemyimg3.jpg      # 敌机2图片
│   ├── bulimg2.jpg        # 子弹图片
│   ├── bkimg2.jpg         # 背景图片
│   └── 音效文件.mp3/.wav  # 各类音效
├── README.md              # 项目说明文档
└── resources.txt          # 资源文件清单
