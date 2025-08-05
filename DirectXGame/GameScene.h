#pragma once
#include "KamataEngine.h"
#include "Player.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Skydome.h"
#include "Fade.h"
#include "MapChipField.h"
#include "CameraController.h"
#include <vector>

static const uint32_t kWindowWidth = 1280;
static const uint32_t kWindowHeight = 720;

// ゲームシーン
class GameScene {
private:
	// ゲームのフェーズ（型）
	enum class Phase {
		kFadeIn, // フェードイン
		kPlay,   // ゲームプレイ
		kDeath,  // デス演出
		kFadeOut,// フェードアウト
	};

	// ゲームの現在フェーズ（変数）
	Phase phase_ = Phase::kFadeIn;

	// フェード用オブジェクト
	Fade* fade_ = nullptr;
	static inline const float kFadeTime = 2.0f;

	// テクスチャハンドル

	// 3Dモデルデータ
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* modelAttack_ = nullptr;
	KamataEngine::Model* modelParticle_ = nullptr;
	KamataEngine::Model* modelBlock_ = nullptr;
	KamataEngine::Model* modelSkydome_ = nullptr;
	KamataEngine::Model* modelEnemy_ = nullptr;

	// カメラ
	KamataEngine::Camera camera_;

	// 天球
	Skydome* skydome_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_;

	// 自キャラ
	Player* player_ = nullptr;

	DeathParticles* deathParticles_ = nullptr;

	// 敵キャラ
	std::list<Enemy*> enemies_;
	// 敵の数
	static inline const int32_t kEnemyNum = 3;

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	CameraController* cameraController_ = nullptr;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;
	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// 終了フラグ
	bool finished_ = false;

public:
	// 初期化
	void Initialize();

	// デストラクタ
	~GameScene();

	// 更新
	void Update();

	// 描画
	void Draw();

	// デスフラグのgetter
	bool IsFinished() const {
		return finished_;
	}

private:
	void GenarateBlocks();

	// 全ての当たり判定を行う
	void CheckAllCollisions();

	void ChangePhase();
};
