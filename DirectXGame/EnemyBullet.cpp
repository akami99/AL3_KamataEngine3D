#include "EnemyBullet.h"
#include "EngineMathFunctions.h"
#include "WorldTransform.h"
#include <cassert>

using namespace KamataEngine;

void EnemyBullet::Initialize(Model *model, const Vector3 &position, const Vector3 &velocity) {
	assert(model);
	model_ = model;
	velocity_ = velocity;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	// 弾のサイズ調整
	worldTransform_.scale_ = { 0.5f, 0.5f, 0.5f };

	// 死亡フラグをリセット
	isDead_ = false;

	// 寿命タイマーをリセット
	deathTimer_ = kLifeTime;

	// 初期の向きを速度ベクトルに合わせる
	worldTransform_.rotation_.y = std::atan2f(velocity_.x, velocity_.z);

	// ワールド変換の更新
	UpdateWorldTransform(worldTransform_);
}

void EnemyBullet::Update() {
	// 位置更新
	worldTransform_.translation_ += velocity_;
	// ワールド変換の更新
	UpdateWorldTransform(worldTransform_);
	// 寿命タイマーをデクリメント
	if (--deathTimer_ <= 0) {
		isDead_ = true;
	}
}

void EnemyBullet::Draw(const Camera &camera) {
	if (isDead_) {
		return; // 死亡している場合は描画しない
	}
	// モデルの描画
	model_->Draw(worldTransform_, camera);
}