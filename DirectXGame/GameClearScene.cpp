#include "GameClearScene.h"
#include "EngineMathFunctions.h"
#include "WorldTransform.h"
#include <numbers>
#include <algorithm>

using namespace KamataEngine;

void GameClearScene::Initialize() {
	// 3Dモデルデータの生成
	modelClearName_ = Model::CreateFromOBJ("clear", true);
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	modelBackGround_ = Model::CreateFromOBJ("background", true);

	worldTransformClear_.Initialize();
	worldTransformClear_.translation_ = Vector3{ 0.0f, 2.0f, 0.0f };

	worldTransformBackGround1_.Initialize();
	worldTransformBackGround1_.translation_ = Vector3{ -30.0f, -15.0f, 20.0f };

	worldTransformBackGround2_.Initialize();
	worldTransformBackGround2_.translation_ = Vector3{ 0.0f, -15.0f, 20.0f };

	worldTransformBackGround3_.Initialize();
	worldTransformBackGround3_.translation_ = Vector3{ 30.0f, -15.0f, 20.0f };

	// カメラの初期化
	camera_.Initialize();
	camera_.farZ = 1000.0f;
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

GameClearScene::~GameClearScene() {
	// 3Dモデルデータの解放
	delete modelClearName_;
	delete modelSkydome_;
	delete modelBackGround_;

	delete fade_;
	delete skydome_;
}

void GameClearScene::Update() {
	float t = 0.0f;

	// 天球の更新
	skydome_->Update();

	// スペースキーを押すとフェードアウトを開始する
	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
		fade_->Start(Fade::Status::FadeOut, kFadeTime);
		phase_ = Phase::kFadeOut;
	}

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
			fade_->Initialize();
		}
		break;

	case Phase::kMain:

		break;

	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	}

	// タイトルと同じく、Clearの文字をアニメーションさせる
	counter_ += returnT_ * (1.0f / 60.0f);
	if (counter_ >= kDuration || counter_ <= 0.0f) {
		returnT_ *= -1.0f;
	}
	t = counter_ / kDuration;
	worldTransformClear_.translation_.y = std::clamp((1.0f - t), 0.0f, 1.0f) + kOriginePos;

	UpdateWorldTransform(worldTransformClear_);
	camera_.UpdateMatrix();
}

void GameClearScene::Draw() {
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

	// Clearモデルの描画
	modelClearName_->Draw(worldTransformClear_, camera_);

	// 3Dモデル描画後処理
	Model::PostDraw();
	// フェードの描画
	if (phase_ == Phase::kFadeIn || phase_ == Phase::kFadeOut) {
		Sprite::PreDraw(dxCommon->GetCommandList());
		fade_->Draw();
		Sprite::PostDraw();
	}
}