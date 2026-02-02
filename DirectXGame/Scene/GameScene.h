#pragma once
#include "KamataEngine.h"
#include "DeathParticles.h"
#include "Skydome.h"
#include "Fade.h"
#include "CameraController.h"
#include "Stage.h"
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

	// 現在のステージ番号 (1からスタート)
	int stageNo_ = 1;

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
	KamataEngine::Model* modelEnemyBullet_ = nullptr;
	KamataEngine::Model* modelDoor_ = nullptr;

	// カメラ
	KamataEngine::Camera camera_;

	// 天球
	Skydome* skydome_ = nullptr;

	// ステージ
	Stage* stage_ = nullptr;

	DeathParticles* deathParticles_ = nullptr;

	// 敵の数
	static inline const int32_t kEnemyNum = 16;

	const float kBlockSize_ = 2.0f; // ブロックのサイズ(2.0f)
	const float kBlockHalfSize_ = kBlockSize_ * 0.5f;

	CameraController* cameraController_ = nullptr;

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
	void Initialize(int stageNo);

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
	/// 衝突判定関数群
	// 全ての当たり判定を行う
	void CheckAllCollisions();
	// プレイヤーと敵の衝突判定
	void CheckPlayerAndEnemies();
	// 敵の弾の衝突判定
	void CheckEnemyBullets();
	// プレイヤーの攻撃の衝突判定
	void CheckPlayerAttack();
	// プレイヤーとドアの衝突判定
	void CheckPlayerAndDoor();
	// ブロックと自キャラの衝突判定
	void CheckPlayerAndBlocks();

	// ステージ読み込み関数を独立させる
	void LoadStage();

	void ChangePhase();
};
