#include "Door.h"
#include "WorldTransform.h"
#include "EngineMathFunctions.h"

using namespace KamataEngine;

void Door::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// NULLポインタチェック
	assert(model);

	// 引数の内容をメンバ変数に記録
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
}

void Door::Update() {
	// 行列計算
	UpdateWorldTransform(worldTransform_);
}

void Door::Draw() {
	model_->Draw(worldTransform_, *camera_);
}

void Door::OnCollision() {
	// プレイヤーがドアに触れた場合の処理
	// このコードでは、GameScene::CheckAllCollisions()でクリア判定を行うため、ここでは何も行わない
}

KamataEngine::Vector3 Door::GetWorldPosition() {
	Vector3 worldPos;
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

AABB Door::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;
	aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kWidth / 2.0f, worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kWidth / 2.0f, worldPos.z + kWidth / 2.0f };
	return aabb;
}