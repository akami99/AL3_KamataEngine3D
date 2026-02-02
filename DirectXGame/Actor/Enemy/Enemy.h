#pragma once

#include "EnemyBullet.h"
#include "EngineMath.h"
#include "KamataEngine.h"
#include <list>

class Player;

class Enemy {
public:
	// 敵の行動タイプ
	enum class Type {
		kWark,  // 歩行
		kShoot, // 射撃
	};

	~Enemy();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <param name="camera">カメラ</param>
	/// <param name="position">初期位置</param>
	void Initialize(KamataEngine::Model* model, const KamataEngine::Vector3& position, Type type = Type::kWark);

	/// <summary>
	/// 更新
	/// <summary>
	void Update();

	/// <summary>
	/// 描画
	/// <summary>
	void Draw(const KamataEngine::Camera& camera);

	// プレイヤー情報をセット
	void SetPlayer(Player* player) { player_ = player; }
	// 弾のモデルをセット
	void SetBulletModel(KamataEngine::Model* model) { bulletModel_ = model; }

	// 衝突応答
	void OnCollision(const Player* player);

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition();

	// AABBを取得
	AABB GetAABB();

	// デス判定を取得
	bool IsDead() const { return isDead_; }
	// デス処理
	void OnSlay() { isDead_ = true; }

	// 弾のリストを取得
	const std::list<EnemyBullet*>& GetBullets() const { return bullets_; }

private:
	// 行動タイプごとの更新処理
	void UpdateWalk();
	void UpdateShoot();

	// 発射処理
	void Fire();

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
	// プレイヤー
	Player* player_ = nullptr;

	// キャラクターの当たり判定サイズ(実際に使う際に調整する部分)
	static inline const float kWidth = 0.9f;
	static inline const float kHeight = 0.9f;

	// タイプ
	Type type_ = Type::kWark;

	// --- 歩行用パラメータ ---
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
	// 最初の角度[度]
	static inline const float kWalkMotionAngleStart = -10.0f;
	// 最後の角度[度]
	static inline const float kWalkMotionAngleEnd = 10.0f;
	// アニメーションの周期となる時間[秒]
	static inline const float kWalkMotionTime = 1.0f;
	// 経過時間
	float walkTimer_ = 0.0f;

	//// 旋回開始時の角度
	// float turnFirstRotationY_ = 0.0f;
	//// 旋回タイマー
	// float turnTimer_ = 0.0f;
	//// 旋回時間<秒>
	// static inline const float kTimeTurn = 0.3f;

	// --- 射撃用パラメータ ---
	// 弾の管理
	std::list<EnemyBullet*> bullets_;
	KamataEngine::Model* bulletModel_ = nullptr;
	// 射撃タイマー
	int fireTimer_ = 0;
	// 射撃間隔フレーム数
	static const int kFireInterval = 120; // 2秒に1回
	// 弾速
	static inline const float kBulletSpeed = 0.2f;

	// デスフラグ
	bool isDead_ = false;
};
