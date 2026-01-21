#include "Enemy.h"
#include "EngineMathFunctions.h"
#include "Player.h"
#include "WorldTransform.h"
#include <cmath>
#include <numbers>

using namespace KamataEngine;

Enemy::~Enemy() {
	// 弾の解放
	for (EnemyBullet* bullet : bullets_) {
		delete bullet;
	}
	bullets_.clear();
}

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position, Type type) {
#ifdef _DEBUG

	// NULLポインタチェック
	assert(model);

#endif // _DEBUG
	if (!model) {
		return; // または適切なエラーハンドリング
	}

	// 引数の内容をメンバ変数に記録
	model_ = model;
	camera_ = camera;
	type_ = type;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	// worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// タイプに応じた初期化
	if (type_ == Type::kWark) {
		// 速度を設定する
		velocity_ = Vector3{-kWalkSpeed, 0.0f, 0.0f};
		lrDirection_ = LRDirection::kLeft;
	} else {
		// 射撃タイプは移動しない（必要なら設定する）
		velocity_ = {0.0f, 0.0f, 0.0f};
	}
	walkTimer_ = 0.0f;
	isDead_ = false;
	fireTimer_ = 0;
}

void Enemy::Update() {

	switch (type_) {
	case Type::kWark:
		UpdateWalk();
		break;
	case Type::kShoot:
		UpdateShoot();
		break;
	}

	// 弾の更新
	for (EnemyBullet* bullet : bullets_) {
		bullet->Update();
	}
	// 死亡した弾を削除
	bullets_.remove_if([](EnemyBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	// 行列計算
	UpdateWorldTransform(worldTransform_);
}

void Enemy::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_);

	// 弾の描画
	for (EnemyBullet* bullet : bullets_) {
		bullet->Draw(*camera_);
	}
}

void Enemy::OnCollision(const Player* player) { (void)player; }

KamataEngine::Vector3 Enemy::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得（ワールド行列）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Enemy::UpdateWalk() {
	// 移動
	worldTransform_.translation_ += velocity_;

	// 巡回タイマーを加算
	patrolTimer_ += 1.0f / 60.0f;

	// 回転アニメーション
	float param = std::sin(2 * std::numbers::pi_v<float> * walkTimer_ / kWalkMotionTime);
	// paramを0.0f～1.0fの範囲に変換し、これを係数tとする
	float t = (param + 1.0f) / 2.0f;
	float degree = Lerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, t);
	worldTransform_.rotation_.x = degree * (std::numbers::pi_v<float> / 180.0f);

	// タイマーを加算
	walkTimer_ += 1.0f / 60.0f;

	// 巡回時間を超えたら方向転換
	if (patrolTimer_ >= kTimePatrol) {
		// タイマーをリセット
		patrolTimer_ = 0.0f;
		// 反転
		if (lrDirection_ == LRDirection::kRight) {
			lrDirection_ = LRDirection::kLeft;
			velocity_.x = -kWalkSpeed;
		} else {
			lrDirection_ = LRDirection::kRight;
			velocity_.x = kWalkSpeed;
		}
	}

	// 旋回制御
	// 左右のキャラ角度テーブル
	float destinationRotationYTable[] = {
	    std::numbers::pi_v<float> / 2.0f,        // 右
	    std::numbers::pi_v<float> * 3.0f / 2.0f, // 左
	};
	// 状況に応じた角度を設定する
	worldTransform_.rotation_.y = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
}

void Enemy::UpdateShoot() {
	// プレイヤーがいない、または死亡中なら何もしない
	if (!player_ || player_->IsDead()) {
		return;
	}

	// プレイヤーの方を向く
	Vector3 playerPos = player_->GetTranslation();
	Vector3 myPos = worldTransform_.translation_;

	// 敵からプレイヤーへのベクトル
	Vector3 diff = playerPos - myPos;

	// Y軸回りの角度を計算
	worldTransform_.rotation_.y = std::atan2f(diff.x, diff.z);

	// 射撃タイマーをカウントアップ
	fireTimer_++;
	if (fireTimer_ >= kFireInterval) {
		// 射撃
		Fire();
		// タイマーリセット
		fireTimer_ = 0;
	}
}

void Enemy::Fire() {
	if (!bulletModel_ || !player_) {
		return;
	}

	// 発射時のプレイヤー位置をターゲットにする
	Vector3 playerPos = player_->GetTranslation();
	Vector3 myPos = worldTransform_.translation_;

	// 速度ベクトルを計算
	Vector3 velocity = playerPos - myPos;

	// ベクトルの正規化
	velocity = Normalize(velocity);
	// 速度に弾速を掛ける
	velocity *= kBulletSpeed;

	// 弾を生成してリストに追加
	EnemyBullet* newBullet = new EnemyBullet();
	newBullet->Initialize(bulletModel_, myPos, velocity);
	bullets_.push_back(newBullet);
}
