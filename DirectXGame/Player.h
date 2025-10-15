#pragma once

#include "KamataEngine.h"
#include "EngineMath.h"

class Enemy;

class MapChipField;

/// <summary>
/// 自キャラ
/// </summary>
class Player {
private:
	// 方向
	enum class Direction {
		kRight, // 右
		kLeft,  // 左
		kFront, // 正面
		kBack,  // 背面
	};

	// 振る舞い
	enum class Behavior {
		kUnknown = 0,
		kRoot,  // 通常状態
		kAttack,// 攻撃中
	};

	// 振る舞い
	Behavior behavior_ = Behavior::kRoot;
	// 次の振る舞いリクエスト
	Behavior behaviorRequest_ = Behavior::kUnknown;

	// 攻撃フェーズ（型）
	enum class AttackPhase {
		Charge,     // 溜め
		Rush,       // 突進
		Aftertaste, // 余韻
	};

	// 現在の攻撃フェーズ（変数）
	AttackPhase attackPhase_;

	// 角
	enum class Corner {
		kRightBottom,    // 右下
		kLeftBottom,     // 左下
		kRightTop,       // 右上
		kLeftTop,        // 左上

		kNumCorner       // 要素数

	};

	// マップとの当たり判定情報
	struct CollisionMapinfo {
		bool onCollisionCeiling_ = false; // 天井に衝突しているか
		bool onCollisionGround_ = false;   // 床に衝突しているか(着地しているか)
		bool onCollisionWall_ = false;    // 左側に衝突しているか
		KamataEngine::Vector3 moveAmount_;      // 移動量
	};

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::WorldTransform worldTransformAttack_;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* modelAttack_ = nullptr;
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;
	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// 速度
	KamataEngine::Vector3 velocity_ = {};
	// 加速度
	static inline const float kAcceleration = 0.05f;
	// 減速度
	static inline const float kAttenuation = 0.08f;
	// 移動速度の上限
	static inline const float kLimitRunSpeed = 0.2f;
	// 移動方向
	Direction direction_ = Direction::kRight;

	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;
	// 旋回時間<秒>
	static inline const float kTimeTurn = 0.3f;

	// 接地状態フラグ
	bool onGround_ = true;

	// 重力加速度（下方向）
	static inline const float kGravityAcceleration = 0.004f;
	// 最大落下速度（下方向）
	static inline const float kLimitFallSpeed = 0.08f;
	// ジャンプ初速（上方向）
	static inline const float kJumpAcceleration = 0.1f;

	// キャラクターの当たり判定サイズ(実際に使う際に調整する部分)
	static inline const float kWidth = 0.9f;
	static inline const float kHeight = 0.9f;

	static inline const float kBlank = 0.01f; // 当たり判定のブランク値
	static inline const float kCollisionOffsetBottom = 0.01f;

	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.4f;
	static inline const float kAttenuationWall = 1.0f;

	// 攻撃ギミックの経過時間カウンター
	float  attackParameter_ = 0.0f;
	static inline const float kChargeTime = 0.1f;
	static inline const float kRushTime = 0.02f;
	static inline const float kAftertasteTime = 0.4f;

	// デスフラグ
	bool isDead_ = false;

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="textureHandle">テクスチャハンドル</param>
	/// <param name="camera">カメラ</param>
	void Initialize(KamataEngine::Model* model, KamataEngine::Model* modelAttack, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// <summary>
	/// 更新
	/// <summary>
	void Update();

	/// <summary>
	/// 描画
	/// <summary>
	void Draw();

	// 衝突応答
	void OnCollision(const Enemy* enemy);

	const KamataEngine::WorldTransform& GetWorldTransform() const {
		return worldTransform_;
	}

	const KamataEngine::Vector3& GetVelocity() const {
		return velocity_;
	}

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition();

	// AABBを取得
	AABB GetAABB();

	// デスフラグのgetter
	bool IsDead() const {
		return isDead_;
	}

	void SetMapChipField(MapChipField* mapChipField) {
		mapChipField_ = mapChipField;
	}

private:
	// 通常行動更新
	void BehaviorRootUpdate();

	// 攻撃行動更新
	void BehaviorAttackUpdate();

	// 通常行動初期化
	void BehaviorRootInitialize();

	// 攻撃行動初期化
	void BehaviorAttackInitialize();

	/// <summary>
	/// 移動処理
	/// </summary>
	void Move();

	/// <summary>
	/// マップの衝突情報をチェック
	/// </summary>
	/// <param name="info">衝突判定に使用するマップ情報への参照</param>
	void MapCollisionCheck(CollisionMapinfo& info);

	/// <summary>
	/// 上方向のマップ衝突判定
	/// </summary>
	/// <param name="info">衝突判定に使用するCollisionMapinfo構造体への参照</param>
	void MapCollisionCheckUp(CollisionMapinfo& info);

	/// <summary>
	/// 下方向のマップ衝突判定
	/// </summary>
	/// <param name="info">衝突判定に使用するCollisionMapinfo構造体への参照</param>
	void MapCollisionCheckDown(CollisionMapinfo& info);

	/// <summary>
	/// 右方向のマップ衝突判定
	/// </summary>
	/// <param name="info">衝突判定に使用するCollisionMapinfo構造体への参照</param>
	void MapCollisionCheckRight(CollisionMapinfo& info);

	/// <summary>
	/// 左方向のマップ衝突判定
	/// </summary>
	/// <param name="info">衝突判定に使用するCollisionMapinfo構造体への参照</param>
	void MapCollisionCheckLeft(CollisionMapinfo& info);

	/// <summary>
	/// 中心座標とコーナー種別から、指定されたコーナーの座標を計算
	/// </summary>
	/// <param name="center">基準となる中心の座標（KamataEngine::Vector3 型）</param>
	/// <param name="corner">取得したいコーナーの種類を指定する Corner 列挙型</param>
	/// <returns>指定されたコーナーの座標（KamataEngine::Vector3 型）</returns>
	KamataEngine::Vector3 CornerPosition(KamataEngine::Vector3 center, Corner corner);

	/// <summary>
	/// 指定された基準位置から、すべてのコーナーの位置を計算して返す
	/// </summary>
	/// <param name="basePosition">コーナー位置の計算に使用する基準となる3次元ベクトル</param>
	/// <returns>各コーナーの位置を格納した std::array<KamataEngine::Vector3, static_cast<uint32_t>(Corner::kNumCorner)></returns>
	std::array<KamataEngine::Vector3, static_cast<uint32_t>(Corner::kNumCorner)>
		CalculateCornerPositions(const KamataEngine::Vector3& basePosition);

	/// <summary>
	/// 衝突結果を適用
	/// </summary>
	/// <param name="info">衝突情報を含む CollisionMapinfo 型の参照</param>
	void ApplyCollisionResult(const CollisionMapinfo& info);

	/// <summary>
	/// 天井に接触している際の処理
	/// </summary>
	/// <param name="info">衝突情報を含む CollisionMapinfo 型の参照</param>
	void OnCollisionCeiling(const CollisionMapinfo& info);

	/// <summary>
	/// 壁に接触している際の処理
	/// </summary>
	/// <param name="info">衝突情報を含む CollisionMapinfo 型の参照</param>
	void OnCollisionWall(const CollisionMapinfo& info);

	/// <summary>
	/// 接地状態の切り替え処理
	/// </summary>
	/// <param name="info">衝突情報を含む CollisionMapinfo 型の参照</param>
	void OnGroundSwitch(const CollisionMapinfo& info);
};

