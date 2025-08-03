#pragma once

#include "KamataEngine.h"

/// <summary>
/// 自キャラ
/// </summary>
class Player {
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

	// 速度
	KamataEngine::Vector3 velocity_ = {};
	// 加速度
	static inline const float kAcceleration = 0.01f;
	// 減速度
	static inline const float kAttenuation = 0.08f;
	// 移動速度の上限
	static inline const float kLimitRunSpeed = 0.2f;
	// 移動方向
	LRDirection lrDirection_ = LRDirection::kRight;

	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;
	// 旋回時間<秒>
	static inline const float kTimeTurn = 0.3f;

	// 接地状態フラグ
	bool onGround_ = true;

	// 重力加速度（下方向）
	static inline const float kGravityAcceleration = 0.045f;
	// 最大落下速度（下方向）
	static inline const float kLimitFallSpeed = 1.0f;
	// ジャンプ初速（上方向）
	static inline const float kJumpAcceleration = 0.6f;

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
};
