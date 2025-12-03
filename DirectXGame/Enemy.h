#pragma once

#include "KamataEngine.h"
#include "EngineMath.h"

class Player;

class Enemy {
private:
	// 左右
	enum class LRDirection {
		kRight,
		kLeft,
	};

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// キャラクターの当たり判定サイズ(実際に使う際に調整する部分)
	static inline const float kWidth = 0.9f;
	static inline const float kHeight = 0.9f;

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.05f;
	// 速度
	KamataEngine::Vector3 velocity_ = {};
	// 移動方向
	LRDirection lrDirection_ = LRDirection::kLeft;

	// 巡回タイマー
	float patrolTimer_ = 0.0f;
	// 巡回時間[秒]
	static inline const float kTimePatrol = 3.0f; // 3秒ごとに方向転換する

	//// 旋回開始時の角度
	//float turnFirstRotationY_ = 0.0f;
	//// 旋回タイマー
	//float turnTimer_ = 0.0f;
	//// 旋回時間<秒>
	//static inline const float kTimeTurn = 0.3f;

	// 最初の角度[度]
	static inline const float kWalkMotionAngleStart = -10.0f;
	// 最後の角度[度]
	static inline const float kWalkMotionAngleEnd = 10.0f;
	// アニメーションの周期となる時間[秒]
	static inline const float kWalkMotionTime = 1.0f;
	// 経過時間
	float walkTimer_ = 0.0f;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <param name="camera">カメラ</param>
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// <summary>
	void Update();

	/// <summary>
	/// 描画
	/// <summary>
	void Draw();

	// 衝突応答
	void OnCollision(const Player* player);

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition();

	// AABBを取得
	AABB GetAABB();
};

