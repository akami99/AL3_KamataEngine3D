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

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, kFadeTime);
}

TitleScene::~TitleScene() {
	// 3Dモデルデータの解放
	delete model_;
	delete modelTitleName_;
	delete fade_;
}

void TitleScene::Update() {
	float t = 0.0f;
	switch (phase_) {
	case Phase::kFadeIn: // フェードインフェーズ
		fade_->Update();
		if (fade_->IsFinished()) {
			// フェードインが終了したらメインフェーズに切り替え
			phase_ = Phase::kMain;
			fade_->Initialize(); // フェードの値をリセット
		}
		break;

	case Phase::kMain: // メインフェーズ
		// メイン部ではスペースキーを押したらフェードアウトを開始する
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			fade_->Start(Fade::Status::FadeOut, kFadeTime); // フェードアウト開始
			phase_ = Phase::kFadeOut;
		}

		// タイトルアニメーションなど、メインの更新処理
		counter_ += returnT_ * (1.0f / 60.0f);
		if (counter_ >= kDuration || counter_ <= 0.0f) {
			returnT_ *= -1.0f;
		}
		t = counter_ / kDuration;
		worldTransformTitle_.translation_.y = std::clamp((1.0f - t), 0.0f, 1.0f) + kOriginePos;

		break;

	case Phase::kFadeOut: // フェードアウトフェーズ
		fade_->Update();
		if (fade_->IsFinished()) {
			// フェードアウトが終了したらタイトルシーンを終了する
			finished_ = true;
		}
		break;
	}

	UpdateWorldTransform(worldTransform_);
	UpdateWorldTransform(worldTransformTitle_);
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
	// フェードイン中/フェードアウト中はフェードの描画を行う
	if (phase_ == Phase::kFadeIn || phase_ == Phase::kFadeOut) {
		Sprite::PreDraw(dxCommon->GetCommandList());
		fade_->Draw();
		Sprite::PostDraw();
	}
}