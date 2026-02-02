#include "Fade.h"
#include <algorithm>

using namespace KamataEngine;

void Fade::Initialize() {
	static const uint32_t kWindowWidth = 1280;
	static const uint32_t kWindowHeight = 720;

	uint32_t textureHandke_ = TextureManager::Load("./Resources/title/fade.png");

	sprite_ = Sprite::Create(textureHandke_, { 0.0f, 0.0f });
	sprite_->SetSize(Vector2(kWindowWidth, kWindowHeight));
	sprite_->SetColor(Vector4(0, 0, 0, 1));
}

Fade::~Fade() {
	delete sprite_;
	sprite_ = nullptr;
}

void Fade::Update() {
	switch (status_) {
	case Status::None:
		// 何もしない
		break;
	case Status::FadeIn:
		// １フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 0.0fから1.0fの間で、経過時間がフェード継続時間に近くほどアルファ値を大きくする
		sprite_->SetColor(Vector4(0, 0, 0, std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f)));
		break;
	case Status::FadeOut:
		// １フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 0.0fから1.0fの間で、経過時間がフェード継続時間に近くほどアルファ値を大きくする
		sprite_->SetColor(Vector4(0, 0, 0, std::clamp(counter_ / duration_, 0.0f, 1.0f)));
		break;
	}
}

void Fade::Draw() {
	if (status_ == Status::None) {
		return;
	}

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Sprite::PreDraw(dxCommon->GetCommandList());

	sprite_->Draw();

	Sprite::PostDraw();
}

void Fade::Start(Status status, float duration) {
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;

	switch (status_) {
	case Status::FadeIn:
		sprite_->SetColor(Vector4(0, 0, 0, 1));
		break;
	case Status::FadeOut:
		sprite_->SetColor(Vector4(0, 0, 0, 0));
		break;
	}
}

void Fade::Stop() {
	status_ = Status::None;
}

bool Fade::IsFinished() const {
	// フェード状態による分岐
	switch (status_) {
	case Status::FadeIn:
	case Status::FadeOut:
		if (counter_ >= duration_) {
			return true;
		} else {
			return false;
		}
	}

	return true;
}