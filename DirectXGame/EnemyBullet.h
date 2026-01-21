#pragma once
#include "KamataEngine.h"

/// 敵弾クラス
class EnemyBullet {
public: // メンバ関数
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="position">初期位置</param>
	/// <param name="velocity">速度</param>
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const KamataEngine::Camera& camera);
	// 死亡フラグを取得
	bool IsDead() const { return isDead_; }
	// 衝突時のコールバック
	void OnCollision();

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition();

	public:
		// 弾の判定サイズ
	    static inline const float kRadius = 0.5f;

private: // メンバ変数
	     // ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	// 速度
	KamataEngine::Vector3 velocity_ = {};
	// 死亡フラグ
	bool isDead_ = false;

	// 寿命タイマー
	static const int kLifeTime = 60 * 5; // 5秒
	int deathTimer_ = kLifeTime;
};
