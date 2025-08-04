#include "Enemy.h"
#include "EngineMathFunctions.h"
#include "WorldTransform.h"
#include <numbers>

using namespace KamataEngine;

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// NULLポインタチェック
	assert(model);

	// 引数の内容をメンバ変数に記録
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// 速度を設定する
	velocity_ = Vector3{ -kWalkSpeed, 0.0f, 0.0f };

	walkTimer_ = 0.0f;
}

void Enemy::Update() {
	// 移動
	worldTransform_.translation_ += velocity_;

	// 旋回制御
	// 左右の自キャラ角度テーブル
	float destinationRotationYTable[] = {
		std::numbers::pi_v<float> / 2.0f, // 右
		std::numbers::pi_v<float> *3.0f / 2.0f, // 左
	};
	// 状況に応じた角度を取得する
	float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
	// 自キャラの角度を設定する
	worldTransform_.rotation_.y = destinationRotationY;
	
	// タイマーを加算
	walkTimer_ += 1.0f / 60.0f;

	// 回転アニメーション
	float param = std::sin(2 * std::numbers::pi_v<float> *walkTimer_ / kWalkMotionTime);
	// paramを0.0f～1.0fの範囲に変換し、これを係数tとする
	float t = (param + 1.0f) / 2.0f;
	float degree = Lerp(kWalkMotionAngleStart, kWalkMotionAngleEnd, t);
	worldTransform_.rotation_.x = degree * (std::numbers::pi_v<float> / 180.0f);

	// 行列計算
	UpdateWorldTransform(worldTransform_);
}

void Enemy::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_);
}