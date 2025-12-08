#pragma once
#include "KamataEngine.h"
#include "Player.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Skydome.h"
#include "Fade.h"
#include "CameraController.h"
#include "Door.h"
#include <vector>
#include <list>

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
		kClear   // ゲームクリア
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
	KamataEngine::Model* modelDoor_ = nullptr;
	//KamataEngine::Model* modelBackGround_ = nullptr;

	// カメラ
	KamataEngine::Camera camera_;

	// 天球
	Skydome* skydome_ = nullptr;

	// 背景
	/*KamataEngine::WorldTransform worldTransformBackGround1_;
	KamataEngine::WorldTransform worldTransformBackGround2_;
	KamataEngine::WorldTransform worldTransformBackGround3_;
	KamataEngine::WorldTransform worldTransformBackGround4_;*/

	// マップチップフィールド
	//MapChipField* mapChipField_;

	// 自キャラ
	Player* player_ = nullptr;

	DeathParticles* deathParticles_ = nullptr;

	// 敵キャラ
	std::list<Enemy*> enemies_;
	// 敵の数
	static inline const int32_t kEnemyNum = 16;

	const float kBlockSize_ = 2.0f; // ブロックのサイズ(2.0f)
	const float kBlockHalfSize_ = kBlockSize_ * 0.5f;

	// 衝突判定を行う壁・障害物ブロックのリスト
	std::list<KamataEngine::WorldTransform*> collidableBlocks_;

	// 巨大な床ブロック（描画専用）
	KamataEngine::WorldTransform* floorTransform_ = nullptr;

	CameraController* cameraController_ = nullptr;

	// ゴールのドア
	Door* door_ = nullptr;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;
	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// プレイヤー死亡判定
	bool isPlayerDead_ = false;

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

	//プレイヤー死亡判定のgetter
	bool IsPlayerDead() const {
		return isPlayerDead_;
	}

	// デスフラグのgetter
	bool IsFinished() const {
		return finished_;
	}

private:
	void GenarateBlocks();

	void GenarateEnemies(const KamataEngine::Vector3& position);

	// 全ての当たり判定を行う
	void CheckAllCollisions();

	void ChangePhase();
};
