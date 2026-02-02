#include "StageSelectScene.h"
#include "EngineMathFunctions.h"
#include "WorldTransform.h"

using namespace KamataEngine;

// 静的メンバ変数の初期化（最初は0番目＝ステージ1）
int StageSelectScene::currentStageIndex_ = 0;

StageSelectScene::~StageSelectScene() {
	// フェード用オブジェクトの解放
	delete fade_;
	fade_ = nullptr;
	// 天球の解放
	delete skydome_;
	skydome_ = nullptr;
	// 3Dモデルデータの解放
	delete modelSkydome_;
	modelSkydome_ = nullptr;
	delete modelIcon_;
	modelIcon_ = nullptr;
	// メモリ解放
	for (KamataEngine::WorldTransform* transform : iconTransforms_) {
		delete transform;
	}
	iconTransforms_.clear();
}

void StageSelectScene::Initialize() {
	// フェード生成
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	// モデル生成（板モデルなどを読み込む。ここではcubeを代用中）
	modelIcon_ = Model::CreateFromOBJ("cube", true);
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);

	// カメラ初期化
	camera_.Initialize();
	camera_.translation_ = { 0.0f, 0.0f, -10.0f }; // 少し引いて全体を見る

	// 天球初期化
	skydome_ = new Skydome();
	skydome_->Initialize(modelSkydome_, &camera_);

	// アイコンの配置
	for (int i = 0; i < kStageCount; i++) {
		// 新しい WorldTransform をメモリ上に作成
		KamataEngine::WorldTransform* newTransform = new KamataEngine::WorldTransform();

		// 初期化
		newTransform->Initialize();

		// 必要に応じて座標などを設定
		newTransform->translation_.x = i * kIconGap;

		// ベクターに追加
		iconTransforms_.push_back(newTransform);
	}

	// ===============================================
	// カメラの初期位置合わせ
	// ===============================================
	// 配置が終わった後、カメラのX座標を選択中のステージに合わせておく
	if (!iconTransforms_.empty()) {
		camera_.translation_.x = iconTransforms_[currentStageIndex_]->translation_.x;
	}

	// 行列更新を忘れずに
	camera_.UpdateMatrix();

	// 最初に選択されているアイコンのために回転速度をセット
	currentRotSpeed_ = kSelectRotSpeed;
}

void StageSelectScene::Update() {
	// フェード更新
	fade_->Update();

	skydome_->Update();

	switch (phase_) {
	case Phase::kFadeIn:
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		break;

	case Phase::kMain:
		// --- 選択操作 ---
		if (Input::GetInstance()->TriggerKey(DIK_D)) {
			if (currentStageIndex_ < kStageCount - 1) {
				currentStageIndex_++;
				// 選択した瞬間に回転速度を上げる（ビュンッ！となる演出）
				currentRotSpeed_ = kSelectRotSpeed;
			}
			isRightPressed_ = true;
			KamataEngine::DebugText::GetInstance()->ConsolePrintf("INFO::  D:%d\n", currentStageIndex_);
		} else {
			isRightPressed_ = false;
		}

		if (Input::GetInstance()->TriggerKey(DIK_A)) {
			if (currentStageIndex_ > 0) {
				currentStageIndex_--;
				// 選択した瞬間に回転速度を上げる
				currentRotSpeed_ = kSelectRotSpeed;
			}
			isLeftPressed_ = true;
			KamataEngine::DebugText::GetInstance()->ConsolePrintf("INFO::  A:%d\n", currentStageIndex_);
		} else {
			isLeftPressed_ = false;
		}

		// --- 決定操作 ---
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kFadeOut;
		}
		break;

	case Phase::kFadeOut:
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	}

	// --- アニメーション更新 ---

	// 回転速度を徐々に通常速度に戻す（減衰）
	// 線形補間(Lerp)を使って、現在の速度を目標値(kBaseRotSpeed)に近づける
	currentRotSpeed_ = Lerp(currentRotSpeed_, kBaseRotSpeed, kRotAttenuation);

	for (int i = 0; i < kStageCount; ++i) {
		// 選択中のアイコン
		if (i == currentStageIndex_) {
			// Y軸回転を加算
			iconTransforms_[i]->rotation_.y += currentRotSpeed_;
			// 少し手前に大きく表示（強調）
			iconTransforms_[i]->scale_ = { 1.5f, 1.5f, 1.5f };
		}
		// 非選択のアイコン
		else {
			// ゆっくり定常回転
			iconTransforms_[i]->rotation_.y += kBaseRotSpeed;
			// サイズは通常
			iconTransforms_[i]->scale_ = { 1.0f, 1.0f, 1.0f };
		}

		// 行列更新
		UpdateWorldTransform(*iconTransforms_[i]);
	}

	// ===============================================
	// ★追加: カメラの追従処理
	// ===============================================

	// 1. 目標となるX座標（選択中のアイコンのX座標）を取得
	// ※ worldTransforms_ はポインタのvectorなので -> でアクセス
	float targetCameraX = iconTransforms_[currentStageIndex_]->translation_.x;

	// 2. カメラのX座標を、目標値に向けて滑らかに移動させる (0.1f は追従速度。好みで調整可)
	camera_.translation_.x = Lerp(camera_.translation_.x, targetCameraX, 0.1f);

	// カメラ座標が変わったので行列更新
	camera_.UpdateMatrix();
}

void StageSelectScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());

	// 天球描画
	skydome_->Draw();

	// アイコン描画
	for (int i = 0; i < kStageCount; ++i) {
		// 選択中のものだけ明るくする、などの処理を入れても良い
		modelIcon_->Draw(*iconTransforms_[i], camera_);
	}

	Model::PostDraw();

	// フェード描画
	fade_->Draw();
}