#pragma once
#include "KamataEngine.h"
#include "Player.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Skydome.h"
#include "MapChipField.h"
#include "CameraController.h"
#include <vector>

static const uint32_t kWindowWidth = 1280;
static const uint32_t kWindowHeight = 720;

// ゲームシーン
class GameScene {
private:
	// テクスチャハンドル

	// 3Dモデルデータ
	KamataEngine::Model* model_ = nullptr;
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

public:
	// 初期化
	void Initialize();

	// デストラクタ
	~GameScene();

	// 更新
	void Update();

	// 描画
	void Draw();

private:
	void GenarateBlocks();

	// 全ての当たり判定を行う
	void CheckAllCollisions();
};
