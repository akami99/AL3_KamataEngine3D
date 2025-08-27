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
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	modelBackGround_ = Model::CreateFromOBJ("background", true);

	worldTransform_.Initialize();
	worldTransform_.translation_ = Vector3{ 0.0f, -1.0f, -4.0f };
	worldTransform_.rotation_.y = std::numbers::pi_v<float>;

	worldTransformTitle_.Initialize();
	worldTransformTitle_.translation_ = Vector3{ 0.0f, 2.0f, 0.0f };

	worldTransformBackGround1_.Initialize();
	worldTransformBackGround1_.translation_ = Vector3{ -30.0f, -15.0f, 20.0f };

	worldTransformBackGround2_.Initialize();
	worldTransformBackGround2_.translation_ = Vector3{ 0.0f, -15.0f, 20.0f };

	worldTransformBackGround3_.Initialize();
	worldTransformBackGround3_.translation_ = Vector3{ 30.0f, -15.0f, 20.0f };

	// カメラの初期化
	camera_.Initialize();
	camera_.farZ = 1000.0f; // 遠くのオブジェクトまで描画するためにfarZを大きく設定
	camera_.translation_.z = -10.0f;

	// 天球の生成
	skydome_ = new Skydome();
	// 天球の初期化
	skydome_->Initialize(modelSkydome_, &camera_);

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, kFadeTime);

	UpdateWorldTransform(worldTransformBackGround1_);
	UpdateWorldTransform(worldTransformBackGround2_);
	UpdateWorldTransform(worldTransformBackGround3_);
}

TitleScene::~TitleScene() {
	// 3Dモデルデータの解放
	delete model_;
	delete modelTitleName_;
	delete modelSkydome_;
	delete modelBackGround_;

	delete fade_;
	delete skydome_;
}

void TitleScene::Update() {
	float t = 0.0f;

	// 天球の更新
	skydome_->Update();

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

	// 天球の描画
	skydome_->Draw();

	// 背景の描画
	modelBackGround_->Draw(worldTransformBackGround1_, camera_);
	modelBackGround_->Draw(worldTransformBackGround2_, camera_);
	modelBackGround_->Draw(worldTransformBackGround3_, camera_);

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