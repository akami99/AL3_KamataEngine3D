#include "TitleScene.h"
#include "EngineMathFunctions.h"
#include "WorldTransform.h"
#include <numbers>
#include <algorithm>

using namespace KamataEngine;

void TitleScene::Initialize() {
	// 3Dモデルデータの生成
	model_ = Model::CreateFromOBJ("player", true);
	modelTitleName_ = Model::CreateFromOBJ("title", true);

	worldTransform_.Initialize();
	worldTransform_.translation_ = Vector3{ 0.0f, -1.0f, -4.0f };
	worldTransform_.rotation_.y = std::numbers::pi_v<float>;

	worldTransformTitle_.Initialize();
	worldTransformTitle_.translation_ = Vector3{ 0.0f, 2.0f, 0.0f };

	// カメラの初期化
	camera_.Initialize();
	camera_.farZ = 100.0f; // 遠くのオブジェクトまで描画するためにfarZを大きく設定
	camera_.translation_.z = -10.0f;
}

TitleScene::~TitleScene() {
	// 3Dモデルデータの解放
	delete model_;
	delete modelTitleName_;
}

void TitleScene::Update() {
	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
		finished_ = true;
	}
	counter_ += returnT_ * (1.0f / 60.0f);
	if (counter_ >= kDuration || counter_ <= 0.0f) {
		returnT_ *= -1.0f;
	}

	// パーティクルの寿命に対する進行度を0.0f〜1.0fで計算
	float t = counter_ / kDuration;

	worldTransformTitle_.translation_.y = std::clamp((1.0f - t), 0.0f, 1.0f) + kOriginePos;

	UpdateWorldTransform(worldTransform_);
	UpdateWorldTransform(worldTransformTitle_);

	// ビュープロジェクション行列の転送
	camera_.UpdateMatrix();
}

void TitleScene::Draw() {
	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());


	// 自キャラの描画
	model_->Draw(worldTransform_, camera_);

	modelTitleName_->Draw(worldTransformTitle_, camera_);

	// 3Dモデル描画後処理
	Model::PostDraw();
}