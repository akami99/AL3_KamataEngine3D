#include "Player.h"
#include <cassert>

using namespace KamataEngine;

void Player::Initialize(Model* model, uint32_t textureHandle, Camera* camera) {
	// NULLポインタチェック
	assert(model);

	// 引数の内容をメンバ変数に記録
	model_ = model;
	textureHandle_ = textureHandle;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
};

void Player::Update() {
	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();
};

void Player::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_, textureHandle_);
};