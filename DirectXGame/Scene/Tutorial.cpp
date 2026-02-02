#include "Tutorial.h"
#include "WorldTransform.h"
#include "EngineMathFunctions.h"
#include <algorithm> // std::clamp用

using namespace KamataEngine;

Tutorial::Tutorial() {
}

Tutorial::~Tutorial() {
	// メモリ解放
	for (Model* model : pageModels_) {
		delete model;
	}
	pageModels_.clear();
}

void Tutorial::Initialize() {
	// ワールド変換の初期化
	worldTransform_.Initialize();
}

void Tutorial::AddPage(const std::string& modelName) {
	// モデルを読み込んでリストに追加
	// ※注意: モデル生成は重い処理なので、Initialize時にまとめて行うのが推奨です
	Model* newModel = Model::CreateFromOBJ(modelName, true);
	pageModels_.push_back(newModel);
}

void Tutorial::Open(int index) {
	// 範囲外チェック
	if (index < 0 || index >= pageModels_.size()) return;

	// 初期化
	currentPageIndex_ = index;
	state_ = TutorialState::Opening;
	timer_ = 0.0f;

	// 最初のフレームで座標などが変にならないようスケール0にしておく
	worldTransform_.scale_ = { 0.0f, 0.0f, 0.0f };
	UpdateWorldTransform(worldTransform_);
}

void Tutorial::Close() {
	if (state_ == TutorialState::Hidden) return;
	state_ = TutorialState::Closing;
	timer_ = 0.0f;
}

void Tutorial::Update(const Camera& camera) {
	// 非表示なら何もしない
	if (state_ == TutorialState::Hidden) return;

	// デルタタイム（1/60秒）を加算
	timer_ += 1.0f / 60.0f;

	// 進行度 t (0.0 ～ 1.0) を計算
	float t = std::clamp(timer_ / kAnimTime_, 0.0f, 1.0f);

	switch (state_) {
	case TutorialState::Opening:
		// 出現: 0 -> 1 へスケール変化
		UpdateTransform(EaseOutBack(t), camera);

		if (t >= 1.0f) {
			state_ = TutorialState::Active;
		}
		break;

	case TutorialState::Active:
		// 表示中: スケール1.0で維持
		UpdateTransform(1.0f, camera);

		// 入力処理 (SPACE or Enter or Pad A)
		if (Input::GetInstance()->TriggerKey(DIK_SPACE) ||
			Input::GetInstance()->TriggerKey(DIK_RETURN)) {
			NextPage();
		}
		// スキップ (ESC or Pad B)
		if (Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
			Close();
		}
		break;

	case TutorialState::Closing:
		// 終了: 1 -> 0 へスケール変化
		UpdateTransform(EaseInBack(1.0f - t), camera); // 逆再生

		if (t >= 1.0f) {
			state_ = TutorialState::Hidden;
			worldTransform_.scale_ = { 0.0f, 0.0f, 0.0f }; // 完全に消す
		}
		break;
	}

	// 行列更新
	UpdateWorldTransform(worldTransform_);
}

void Tutorial::Draw(const Camera& camera) {
	if (state_ == TutorialState::Hidden) return;

	// 現在のページを描画
	if (currentPageIndex_ < pageModels_.size()) {
		// モデルを描画（ワールド変換は共通のものを使う）
		pageModels_[currentPageIndex_]->Draw(worldTransform_, camera);
	}
}

void Tutorial::NextPage() {
	currentPageIndex_++;

	if (currentPageIndex_ >= pageModels_.size()) {
		// 全ページ終了
		Close();
		currentPageIndex_ = (int)pageModels_.size() - 1;
	} else {
		// ページ切り替え演出（軽くバウンドさせるなど）
		// ここではシンプルにOpening状態に戻してポヨンと出直す演出にします
		state_ = TutorialState::Opening;
		timer_ = 0.0f;
	}
}

void Tutorial::UpdateTransform(float scaleRatio, const Camera& camera) {
	// 1. スケール設定
	// 板ポリゴンなどを想定して少し大きめに調整（必要に応じて変更）
	float baseScale = 0.5f;
	float s = baseScale * scaleRatio;
	worldTransform_.scale_ = { s, s, s };

	// 2. カメラの目の前に配置
	// カメラの回転情報から前方ベクトルを計算
	Vector3 forward;
	forward.x = std::sin(camera.rotation_.y) * std::cos(camera.rotation_.x);
	forward.y = -std::sin(camera.rotation_.x);
	forward.z = std::cos(camera.rotation_.y) * std::cos(camera.rotation_.x);

	// カメラ座標 + 前方ベクトル * 距離
	float distance = 5.0f;
	worldTransform_.translation_ = camera.translation_ + (forward * distance);

	// 3. 常にカメラの方を向く（ビルボード回転）
	// Y軸回転をカメラと同期させる（簡易ビルボード）
	worldTransform_.rotation_.y = camera.rotation_.y;
	worldTransform_.rotation_.x = camera.rotation_.x;
	worldTransform_.rotation_.z = 0.0f;
}

// イージング関数（動きを滑らかにする）
float Tutorial::EaseOutBack(float x) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;
	return 1.0f + c3 * std::pow(x - 1.0f, 3.0f) + c1 * std::pow(x - 1.0f, 2.0f);
}

float Tutorial::EaseInBack(float x) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;
	return c3 * x * x * x - c1 * x * x;
}