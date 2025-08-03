#pragma once
#include "KamataEngine.h"

// 前方宣言
class Player;

/// <summary>
/// カメラコントローラ
/// </summary>
class CameraController {
private:
	// 矩形
	struct Rect {
		float left = 0.0f;     // 左端
		float right = 0.0f;    // 右端
		float bottom = 0.0f;   // 下端
		float top = 0.0f;      // 上端
	};

	// カメラ
	KamataEngine::Camera& camera_;

	// プレイヤー
	Player* target_ = nullptr;
	
	// 追従対象とカメラの座標の差（オフセット）
	KamataEngine::Vector3 TargetOffset_ = { 0.0f, 0.0f, -15.0f };

	// カメラの移動範囲
	Rect movableArea_ = { 0, 100, 0, 100 }; //変更する
	// カメラの目標座標
	KamataEngine::Vector3 targetPosition_ = { 0.0f, 0.0f, 0.0f };
	// 座標補間割合
	static inline const float kInterpolationRate = 0.1f; // 0.0f ~ 1.0f
	// 速度掛け率
	static inline const float kVelocityBias = 20.0f;
	// 追従対象の各方向へのカメラ移動範囲
	static inline const Rect kMargin = { -10.0f, 10.0f, -10.0f, 10.0f }; //プレイヤーが画面外に行くようだったら変更する

public:
	CameraController(KamataEngine::Camera& camera);

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	void SetTarget(Player* target) {
		target_ = target;
	}

	/// <summary>
	/// カメラの移動範囲のセッター
	/// </summary>
	/// <param name="area">left,right,bottom,top</param>
	void SetMovableArea(KamataEngine::Vector4& area) {
		movableArea_.left = area.x;
		movableArea_.right = area.y;
		movableArea_.bottom = area.z;
		movableArea_.top = area.w;
	}

	void Reset();
};

