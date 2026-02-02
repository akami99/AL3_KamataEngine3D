#include "Skydome.h"

using namespace KamataEngine;

void Skydome::Initialize(Model* model, Camera* camera) {
#ifdef DEBUG

	// NULLポインタチェック
	assert(model);
	assert(camera);

#endif // DEBUG

	if (!model || !camera) {
		return;
	}

	// 引数の内容をメンバ変数に記録
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();

	// 初期位置の設定
	worldTransform_.translation_ = { 0.0f, 0.0f, 0.0f };
}

void Skydome::Update() {
	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();
}

void Skydome::Draw() {
	model_->Draw(worldTransform_, *camera_);
}