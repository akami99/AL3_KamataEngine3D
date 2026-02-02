#pragma once

#include "KamataEngine.h"
#include "EngineMath.h"
#include "Player/Player.h"

class Door {
private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 1.0f;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// <summary>
	void Update();

	/// <summary>
	/// 描画
	/// <summary>
	void Draw(const KamataEngine::Camera& camera);

	// 衝突応答
	void OnCollision();

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition();

	// AABBを取得
	AABB GetAABB();
};
