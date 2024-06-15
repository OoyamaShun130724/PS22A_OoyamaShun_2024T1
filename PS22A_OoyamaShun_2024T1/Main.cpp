# include <Siv3D.hpp> 

namespace constants {
	namespace ball {
		constexpr float SPEED = 300.0;
	}
	namespace brick {
		constexpr Size SIZE = { 40,20 };

		constexpr int X_COUNT = 20;
		constexpr int Y_COUNT = 5;
		constexpr int MAX = X_COUNT * Y_COUNT;
	}
}

class GameManager {
public:
	int Score;
	int Life;
	String GameClearText;
	String GameOverText;
	String LifeText;
	String ScoreText;
	Font LifeFont;
	Font ScoreFont;
	Font GameOverFont;
	Font GameClearFont;
	bool IsPlayGame;

	GameManager()
		:Score(0), Life(4),ScoreText(U"score:{}"_fmt(Score)), ScoreFont{20}, GameClearFont{50},
		GameClearText(), GameOverFont{ 50 }, GameOverText(),
		LifeText(U"残機:{}"_fmt(Life+1)), LifeFont{ 50 },
		IsPlayGame(true) {}
	void Draw() {
		ScoreFont(ScoreText).draw(10, 10);
		GameClearFont(GameClearText).draw(300,400);
		GameOverFont(GameOverText).draw(300, 400);
		LifeFont(LifeText).draw(600, 0);
	}
	void AddScore() {
		Score += 100;
		ScoreText = U"score:{}"_fmt(Score);
	}
	bool TryContinue() {
		if (Life > 0) {
			Life--;
			LifeText = U"残機:{}"_fmt(Life+1);
			return true;
		}
		else{
			GameOver();
			return false;
		}
	}
	void GameClear(){
		GameClearText = U"GameClear";
		IsPlayGame = false;
	}
	void GameOver(){
		GameOverText = U"GameOver";
		IsPlayGame = false;
	}
};
class Ball {
public:
	Vec2 Velocity;
	Circle ball;

	Ball() : Velocity({ 0,-constants::ball::SPEED }), ball(400, 400, 8) {}
	void Draw() {
		ball.draw();
	}
	void Update(GameManager* manager) {
		ball.moveBy(Velocity * Scene::DeltaTime());
		if (ball.y > 600) {
			if (manager->TryContinue()){
				ball.x = Cursor::Pos().x;
				ball.y = 400;
			}
		}
	}

};
class Bricks {
public:
	Rect brickTable[constants::brick::MAX];
	int HitCount;

	Bricks():HitCount(0){
		for (int y = 0; y < constants::brick::Y_COUNT; ++y)
		{
			for (int x = 0; x < constants::brick::X_COUNT; ++x)
			{
				int index = (y * constants::brick::X_COUNT) + x;

				brickTable[index] = Rect{
					x * constants::brick::SIZE.x,
					60 + y * constants::brick::SIZE.y,
					constants::brick::SIZE
				};
			}
		}
	}
	void Draw() {
		for (int i = 0; i < constants::brick::MAX; ++i) {
			brickTable[i].stretched(-1).draw(HSV{ brickTable[i].y - 40 });
		}
	}
	void Intercects(Ball* target, GameManager* manager) {
		using namespace constants::brick;
		// ブロックとの衝突を検知
		for (int i = 0; i < MAX; ++i) {
			auto& refBrick = brickTable[i];

			if (refBrick.intersects(target->ball)) {
				// ブロックの上辺、または底辺と交差
				if (refBrick.bottom().intersects(target->ball)
					|| refBrick.top().intersects(target->ball))
				{
					target->Velocity.y *= -1;
				}
				else // ブロックの左辺または右辺と交差
				{
					target->Velocity.x *= -1;
				}

				// あたったブロックは画面外に出す
				refBrick.y -= 600;

				manager->AddScore();
				HitCount++;
				if (HitCount == constants::brick::Y_COUNT * constants::brick::X_COUNT) {
					manager->GameClear();
				}

				// 同一フレームでは複数のブロック衝突を検知しない
				break;
			}
		}

		// 天井との衝突を検知
		if ((target->ball.y < 0) && (target->Velocity.y < 0)){
			target->Velocity.y *= -1;
		}

		// 壁との衝突を検知
		if (((target->ball.x < 0) && (target->Velocity.x < 0))
			|| ((Scene::Width() < target->ball.x) && (0 < target->Velocity.x))){
			target->Velocity.x *= -1;
		}
	}
};
class Paddle {
public:
	Rect paddle;
	Paddle() :paddle{ Arg::center(Cursor::Pos().x, 500),60,10 } {}
	void Draw() {
		paddle.rounded(3).draw();
	}
	void Intercect(Ball* target) {
		using namespace constants::ball;
		// パドルとの衝突を検知
		if ((0 < target->Velocity.y) && paddle.intersects(target->ball))
		{
			target->Velocity = Vec2{
				(target->ball.x - paddle.center().x) * 10,
				-target->Velocity.y
			}.setLength(constants::ball::SPEED);
		}
	}
	void Update() {
		paddle.x = Cursor::Pos().x;
	}
};


void Main()
{
	Ball ball;
	Bricks bricks;
	Paddle paddle;
	GameManager manager;

	while (System::Update())
	{
#pragma region 更新
		ball.Update(&manager);
		paddle.Update();
#pragma endregion

#pragma region 衝突判定
		bricks.Intercects(&ball, &manager);
		paddle.Intercect(&ball);
#pragma endregion

#pragma region  描画
		ball.Draw();
		bricks.Draw();
		paddle.Draw();
		manager.Draw();
#pragma endregion
	}
}
