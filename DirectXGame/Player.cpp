#define NOMIMAX

#include "Player.h"
#include "EngineMathFunctions.h"
#include "WorldTransform.h"
#include <cassert>
#include <numbers>
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace KamataEngine;

// 最短角度補間関数
float LerpShortestAngle(float a, float b, float t) {
	// 角度の差を計算
	float diff = b - a;

	// 差を-πからπの範囲に正規化
	float pi = std::numbers::pi_v<float>;
	float twoPi = 2.0f * pi;

	// fmodを使って剰余を計算する
	diff = std::fmod(diff, twoPi);
	if (diff > pi) {
		diff -= twoPi; // +180度を超える場合は-360度分を引く
	} else if (diff < -pi) {
		diff += twoPi; // -180度未満の場合は+360度分を足す
	}

	// 現在の角度に、補間係数をかけた差分を加算
	return a + diff * t;
}

void Player::Initialize(Model* model, Model* modelAttack, Camera* camera, const Vector3& position) {
#ifdef _DEBUG

	// NULLポインタチェック
	assert(model);

#endif // _DEBUG
	if (!model) {
		return; // または適切なエラーハンドリング
	}
	// 引数の内容をメンバ変数に記録
	model_ = model;
	modelAttack_ = modelAttack;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	worldTransformAttack_.Initialize();
	worldTransformAttack_.translation_ = position;
	worldTransformAttack_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	worldTransformAttack_.scale_ = { 0.0f, 0.0f, 0.0f }; // 最初は非表示
}

void Player::Update() {
	if (behaviorRequest_ != Behavior::kUnknown) {
		// 振る舞いを変更する
		behavior_ = behaviorRequest_;
		// 各振る舞いごとの初期化を実行

		switch (behavior_) {
			// 通常行動
		case Behavior::kRoot:
		default:
			BehaviorRootInitialize();
			break;
			// 攻撃行動
		case Behavior::kAttack:
			BehaviorAttackInitialize();
			break;
		case Behavior::kDodge:
			BehaviorDodgeInitialize();
			break;
		}
		// 振る舞いリクエストをリセット
		behaviorRequest_ = Behavior::kUnknown;
	}
	switch (behavior_) {
		// 通常行動
	case Behavior::kRoot:
	default:
		BehaviorRootUpdate();
		break;
		// 攻撃行動
	case Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
	case Behavior::kDodge:
		BehaviorDodgeUpdate();
		break;
	}

}

void Player::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_);

	if (attackPhase_ == AttackPhase::Rush) {
		// 攻撃エフェクトモデルを描画
		modelAttack_->Draw(worldTransformAttack_, *camera_);
	}
}

void Player::OnCollision(const Enemy* enemy) {
	// 無敵中なら、何もせず処理を終了
	if (isInvincible_) {
		return;
	}

	(void)enemy;
	isDead_ = true;
}

KamataEngine::Vector3 Player::GetWorldPosition() {
	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得（ワールド行列）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kWidth / 2.0f, worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kWidth / 2.0f, worldPos.z + kWidth / 2.0f };

	return aabb;
}

void Player::BehaviorRootInitialize() {}

void Player::BehaviorAttackInitialize() {
	// カウンター初期化
	attackParameter_ = 0;
	// フェーズを「Charge」に設定
	attackPhase_ = AttackPhase::Charge;
	// 攻撃方向をリセット
	attackDirection_ = {};
	// 攻撃開始時の位置を保存
	originalPosition_ = worldTransform_.translation_;

	// ----- 攻撃方向の決定 -----
	// 現在のキー入力状態から決定する
	Vector3 inputDirection = {};
	if (Input::GetInstance()->PushKey(DIK_D)) {
		inputDirection.x += 1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_A)) {
		inputDirection.x -= 1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_W)) {
		inputDirection.z += 1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_S)) {
		inputDirection.z -= 1.0f;
	}

	// 入力があるかチェック
	if (Length(inputDirection) > 0.0f) {
		// 入力方向を正規化して保存
		attackDirection_ = Normalize(inputDirection);
	} else {
		// 入力が無い場合は、向いている角度から攻撃方向を計算する

		// atan2(x,y)逆で、角度からベクトルを計算
		attackDirection_ = Vector3{
			std::sin(worldTransform_.rotation_.y),
			0.0f,
			std::cos(worldTransform_.rotation_.y)
		};

		// 正規化
		attackDirection_ = Normalize(attackDirection_);
	}
}

void Player::BehaviorDodgeInitialize() {
	// カウンターを初期化
	dodgeParameter_ = 0.0f;
	// フェーズを「Main」に設定
	dodgePhase_ = DodgePhase::Main;
	// 無敵フラグを立てる
	isInvincible_ = true;
	// 回避方向をリセット
	dodgeDirection_ = {};

	// ----- 回避方向の決定 -----
	// 現在のキー入力状態から決定する
	Vector3 inputDirection = {};
	if (Input::GetInstance()->PushKey(DIK_D)) {
		inputDirection.x += 1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_A)) {
		inputDirection.x -= 1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_W)) {
		inputDirection.z += 1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_S)) {
		inputDirection.z -= 1.0f;
	}

	// 入力があるかチェック
	if (Length(inputDirection) > 0.0f) {
		// 入力方向を正規化して保存
		dodgeDirection_ = Normalize(inputDirection);
	} else {
		// 入力が無い場合は、向いている角度から回避方向を計算する

		// atan2(x,y)逆で、角度からベクトルを計算
		dodgeDirection_ = Vector3{
			std::sin(worldTransform_.rotation_.y),
			0.0f,
			std::cos(worldTransform_.rotation_.y)
		};

		// 正規化
		dodgeDirection_ = Normalize(dodgeDirection_);

	}
}

void Player::BehaviorRootUpdate() {

	// 攻撃キーを押したら
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// 攻撃ビヘイビアをリクエスト
		behaviorRequest_ = Behavior::kAttack;
	}

	// 回避キーを押したら
	if (Input::GetInstance()->TriggerKey(DIK_LSHIFT)) {
		// 回避ビヘイビアをリクエスト
		behaviorRequest_ = Behavior::kDodge;
		// 回避中は移動やほかの入力処理を実行させない
		return;
	}

	// 移動入力
	Move();

	// 回転処理
	// 速度ベクトル（移動入力）のXとZの長さ（平面上の速度）をチェック
	float velocityXZLength = std::sqrt(velocity_.x * velocity_.x + velocity_.z * velocity_.z);

	if (velocityXZLength > 0.001f) { // わずかでも移動入力があれば
		// 目標角度を計算（atan2はラジアンを返す）
		float targetAngle = std::atan2(velocity_.x, velocity_.z);

		// 現在の角度を取得
		float currentAngle = worldTransform_.rotation_.y;

		// 最短角度補間で回転させる
		float newAngle = LerpShortestAngle(
			currentAngle,  // 現在の角度
			targetAngle,   // 目標角度
			kRotationSpeed // 補間係数
		);

		// 回転を反映
		worldTransform_.rotation_.y = newAngle;
		// direction_ の更新
		float pi = std::numbers::pi_v<float>;
		if (worldTransform_.rotation_.y > -pi / 4.0f && worldTransform_.rotation_.y <= pi / 4.0f) { // -45度 ～ +45度
			direction_ = Direction::kFront;
		} else if (worldTransform_.rotation_.y > pi / 4.0f && worldTransform_.rotation_.y <= 3.0f * pi / 4.0f) { // +45度 ～ +135度
			direction_ = Direction::kRight;
		} else if (worldTransform_.rotation_.y > 3.0f * pi / 4.0f || worldTransform_.rotation_.y <= -3.0f * pi / 4.0f) { // +135度以上, -135度以下
			direction_ = Direction::kBack;
		} else { // -135度 ～ -45度
			direction_ = Direction::kLeft;
		}
	}

	// 移動量を加味して衝突判定する

	// 衝突情報を初期化
	CollisionMapinfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.moveAmount_ = velocity_;

	// マップ衝突チェック
	//MapCollisionCheck(collisionMapInfo);

	// 判定結果を反映して移動させる
	ApplyCollisionResult(collisionMapInfo);

	// 天井に接触している場合の処理
	OnCollisionCeiling(collisionMapInfo);

	// 壁に接触している場合の処理
	OnCollisionWall(collisionMapInfo);

	// 接地状態の切り替え
	OnGroundSwitch(collisionMapInfo);

	// 行列計算
	UpdateWorldTransform(worldTransform_);
}

void Player::BehaviorAttackUpdate() {
	// 攻撃ギミックの経過時間カウンターを更新
	attackParameter_ += 1.0f / 60.0f; // 1フレームごとに時間を加算

	// --- 回転処理 ---
	if (Length(attackDirection_) > 0.001f) { // わずかでも攻撃方向があれば
		// 目標角度を計算（atan2はラジアンを返す）
		float targetAngle = std::atan2(attackDirection_.x, attackDirection_.z);

		// 現在の角度を取得
		float currentAngle = worldTransform_.rotation_.y;

		// 最短角度補間で回転させる
		float newAngle = LerpShortestAngle(
			currentAngle,  // 現在の角度
			targetAngle,   // 目標角度
			kRotationSpeed // 補間係数
		);
		// 回転を反映
		worldTransform_.rotation_.y = newAngle;

		// direction_ の更新
		float pi = std::numbers::pi_v<float>;
		if (worldTransform_.rotation_.y > -pi / 4.0f && worldTransform_.rotation_.y <= pi / 4.0f) { // -45度 ～ +45度
			direction_ = Direction::kFront;
		} else if (worldTransform_.rotation_.y > pi / 4.0f && worldTransform_.rotation_.y <= 3.0f * pi / 4.0f) { // +45度 ～ +135度
			direction_ = Direction::kRight;
		} else if (worldTransform_.rotation_.y > 3.0f * pi / 4.0f || worldTransform_.rotation_.y <= -3.0f * pi / 4.0f) { // +135度以上, -135度以下
			direction_ = Direction::kBack;
		} else { // -135度 ～ -45度
			direction_ = Direction::kLeft;
		}
	}

	CollisionMapinfo collisionMapInfo;
	Vector3 velocity{};

	switch (attackPhase_) {
	case AttackPhase::Charge:
	default:
	{
		// 溜め動作の更新
		// tは時間の進行度を0.0fから1.0fにする
		float t = std::clamp(attackParameter_ / kChargeTime, 0.0f, 1.0f);
		worldTransform_.scale_.z = EaseOutLerpFloat(1.0f, 0.3f, t); // 画像の指示に合わせてEaseOutに変更
		worldTransform_.scale_.y = EaseOutLerpFloat(1.0f, 1.6f, t); // 画像の指示に合わせてEaseOutに変更

		// 前進動作へと移行
		if (attackParameter_ >= kChargeTime) {
			attackPhase_ = AttackPhase::Rush;
			attackParameter_ = 0.0f; // カウンターリセット
		}
	}
	break;
	case AttackPhase::Rush:
	{
		// 突進動作の更新
		// tは時間の進行度を0.0fから1.0fにする
		float t = std::clamp(attackParameter_ / kRushTime, 0.0f, 1.0f);
		worldTransform_.scale_.z = EaseOutLerpFloat(0.3f, 1.3f, t); // 画像の指示に合わせてEaseOutに変更
		worldTransform_.scale_.y = EaseInLerpFloat(1.6f, 0.7f, t); // 画像の指示に合わせてEaseInに変更

		float length = Length(attackDirection_);
		if (std::abs(length - 1.0f) > 0.001f) {
			// 長さが1.0fからずれている場合は問題あり
			std::cout << "Warning: attackDirection_ is not normalized. Length = " << length << std::endl;
		}

		velocity = attackDirection_ * kAttackSpeed;

		// 余韻動作へと移行
		if (attackParameter_ >= kRushTime) {
			attackPhase_ = AttackPhase::Aftertaste;
			attackParameter_ = 0.0f; // カウンターリセット
		}
	}
	break;
	case AttackPhase::Aftertaste:
	{
		// 余韻動作の更新
		// tは時間の進行度を0.0fから1.0fにする
		float t = std::clamp(attackParameter_ / kAftertasteTime, 0.0f, 1.0f);
		worldTransform_.scale_.z = EaseOutLerpFloat(1.3f, 1.0f, t); // スケールを通常に戻す
		worldTransform_.scale_.y = EaseOutLerpFloat(0.7f, 1.0f, t); // スケールを通常に戻す

		// 元の位置に戻るためのLerp
		Vector3 nextPosition = Lerp(
			originalPosition_,               // 開始位置
			worldTransform_.translation_, // 突進終了位置
			1.0f - t                                 // 0.0f ～ 1.0f の範囲
		);

		// 座標を直接更新する
		worldTransform_.translation_ = nextPosition;

		// 徐々に減速
		/*velocity_.x *= (1.0f - kAttenuation);
		velocity_.z *= (1.0f - kAttenuation);*/
		velocity = Vector3{ 0.0f, 0.0f, 0.0f };

		// ルート動作へと移行
		if (attackParameter_ >= kAftertasteTime) {
			worldTransform_.translation_ = originalPosition_; // 位置を元に戻す
			attackPhase_ = AttackPhase::Charge; // 次の攻撃のためにChargeに戻す
			attackParameter_ = 0.0f; // カウンターリセット
			behaviorRequest_ = Behavior::kRoot; // ルートビヘイビアに戻る
		}
	}
	break;
	}

	// Rushフェーズでのみ衝突判定を通す
	if (attackPhase_ == AttackPhase::Rush) {
		//// 衝突情報を初期化して、velocityを代入
		//collisionMapInfo.moveAmount_ = velocity;
		//// マップ衝突チェック
		//MapCollisionCheck(collisionMapInfo);
		//// 判定結果を反映して移動させる
		//ApplyCollisionResult(collisionMapInfo);

		// 衝突後の速度を次フレームの Rush フェーズのために同期
		velocity = collisionMapInfo.moveAmount_;
	} else if (attackPhase_ == AttackPhase::Aftertaste) {
		if (attackPhase_ == AttackPhase::Aftertaste) {
			// 念のため、Aftertasteフェーズでの移動量を0にする
			velocity = Vector3{ 0.0f, 0.0f, 0.0f };
		}
	}

	// 衝突情報を初期化して、velocityを代入
	//collisionMapInfo.moveAmount_ = velocity;

	// マップ衝突チェック
	//MapCollisionCheck(collisionMapInfo);

	// 判定結果を反映して移動させる
	//ApplyCollisionResult(collisionMapInfo);

	velocity = collisionMapInfo.moveAmount_;

	// 向きに応じて攻撃モデルの向きを変える
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;

	// エフェクト位置の調整
	Vector3 offset = attackDirection_ * kEffectOffsetLength; // 攻撃方向にオフセット

	// トランスフォームに反映
	worldTransformAttack_.translation_ = worldTransform_.translation_ + offset;

	// エフェクトのスケールはフェーズに応じて制御
	if (attackPhase_ == AttackPhase::Rush) {
		// 突進フェーズではフルスケール
		worldTransformAttack_.scale_ = { 1.0f, 1.0f, 1.0f };
	} else {
		// それ以外のフェーズでは非表示
		worldTransformAttack_.scale_ = { 0.0f, 0.0f, 0.0f };
	}

	UpdateWorldTransform(worldTransformAttack_);

	UpdateWorldTransform(worldTransform_);
}

void Player::BehaviorDodgeUpdate() {
	// 回避タイマーを更新
	dodgeParameter_ += 1.0f / 60.0f;

	Vector3 velocity{}; // このフレームでの移動量

	if (Length(dodgeDirection_) > 0.001f) { // わずかでも移動入力があれば
		// 目標角度を計算（atan2はラジアンを返す）
		float targetAngle = std::atan2(dodgeDirection_.x, dodgeDirection_.z);

		// 現在の角度を取得
		float currentAngle = worldTransform_.rotation_.y;

		// 最短角度補間で回転させる
		float newAngle = LerpShortestAngle(
			currentAngle,  // 現在の角度
			targetAngle,   // 目標角度
			kRotationSpeed // 補間係数
		);

		// 回転を反映
		worldTransform_.rotation_.y = newAngle;
		// direction_ の更新
		float pi = std::numbers::pi_v<float>;
		if (worldTransform_.rotation_.y > -pi / 4.0f && worldTransform_.rotation_.y <= pi / 4.0f) { // -45度 ～ +45度
			direction_ = Direction::kFront;
		} else if (worldTransform_.rotation_.y > pi / 4.0f && worldTransform_.rotation_.y <= 3.0f * pi / 4.0f) { // +45度 ～ +135度
			direction_ = Direction::kRight;
		} else if (worldTransform_.rotation_.y > 3.0f * pi / 4.0f || worldTransform_.rotation_.y <= -3.0f * pi / 4.0f) { // +135度以上, -135度以下
			direction_ = Direction::kBack;
		} else { // -135度 ～ -45度
			direction_ = Direction::kLeft;
		}
	}

	// 回避フェーズごとの処理
	switch (dodgePhase_) {
	case DodgePhase::Main:
	default:
	{
		// ----- 回避中フェーズ -----
		// tを 0.0f から 1.0f の範囲にする
		float t = std::clamp(dodgeParameter_ / kDodgeMainTime, 0.0f, 1.0f);

		// 仮の動作として、スケールを変更する
		worldTransform_.scale_.x = EaseInOutLerpFloat(1.0f, 0.5f, t);
		worldTransform_.scale_.y = EaseInOutLerpFloat(1.0f, 1.3f, t);

		// 移動速度をセット
		velocity = dodgeDirection_ * kDodgeSpeed;

		// 硬直動作へと移行
		if (dodgeParameter_ >= kDodgeMainTime) {
			dodgePhase_ = DodgePhase::Aftertaste;
			dodgeParameter_ = 0.0f; // カウンターリセット
		}
	}
	break;
	case DodgePhase::Aftertaste:
	{
		// ----- 硬直フェーズ -----
		// tを 0.0f から 1.0f の範囲にする
		float t = std::clamp(dodgeParameter_ / kDodgeAftertasteTime, 0.0f, 1.0f);

		// 仮の動作として、スケールを元に戻す
		worldTransform_.scale_.x = EaseInOutLerpFloat(0.5f, 1.0f, t);
		worldTransform_.scale_.y = EaseInOutLerpFloat(1.3f, 1.0f, t);

		// ここでは移動しない
		velocity = Vector3{ 0.0f, 0.0f, 0.0f };

		// 硬直時間が終了したらルートビヘイビアに戻る
		if (dodgeParameter_ >= kDodgeAftertasteTime) {
			dodgePhase_ = DodgePhase::Main; // 次の回避のためにMainに戻す
			dodgeParameter_ = 0.0f; // カウンターリセット

			// 無敵フラグを解除
			isInvincible_ = false;
			// ルートビヘイビアに戻る
			behaviorRequest_ = Behavior::kRoot;
		}
	}
	break;
	}
	// ----- 回避中のマップ衝突判定と移動 -----

	// 衝突情報を初期化
	CollisionMapinfo collisionMapInfo;
	// 移動量を設定
	collisionMapInfo.moveAmount_ = velocity;

	// マップ衝突チェック
	//MapCollisionCheck(collisionMapInfo);

	// 判定結果を反映して移動させる
	ApplyCollisionResult(collisionMapInfo);

	// 行列計算
	UpdateWorldTransform(worldTransform_);
}

void Player::Move() {
	// 移動入力
	// 接地状態
	/*if (onGround_) {*/

	// ----------------------------------------------------
	// 入力による加速度ベクトルの計算 (優先度ロジック適用)
	// ----------------------------------------------------
	Vector3 acceleration = {};
	bool inputX = false;
	bool inputZ = false;

	// 入力状態の取得
	bool pushW = Input::GetInstance()->PushKey(DIK_W);
	bool pushS = Input::GetInstance()->PushKey(DIK_S);
	bool pushA = Input::GetInstance()->PushKey(DIK_A);
	bool pushD = Input::GetInstance()->PushKey(DIK_D);

	// --- Z軸（前後）の入力処理 ---
	if (pushW && !pushS) {
		// Wのみ（上方向）
		inputZ = true;
		if (velocity_.z < 0.0f) { velocity_.z *= (1.0f - kAttenuation); } // 急ブレーキ
		acceleration.z += kAcceleration;
	} else if (!pushW && pushS) {
		// Sのみ（下方向）
		inputZ = true;
		if (velocity_.z > 0.0f) { velocity_.z *= (1.0f - kAttenuation); } // 急ブレーキ
		acceleration.z -= kAcceleration;
	}
	// ※ WとSの同時押し (pushW && pushS) の場合は、else ifで拾われないため acceleration.z は 0 のまま

	// 非入力時（W, Sともに押されていない、または W+S の同時押し）はZ軸の移動減衰
	if (!inputZ) {
		velocity_.z *= (1.0f - kAttenuation);
	}

	// --- X軸（左右）の入力処理 ---
	if (pushD && !pushA) {
		// Dのみ（右方向）
		inputX = true;
		if (velocity_.x < 0.0f) { velocity_.x *= (1.0f - kAttenuation); } // 急ブレーキ
		acceleration.x += kAcceleration;
	} else if (!pushD && pushA) {
		// Aのみ（左方向）
		inputX = true;
		if (velocity_.x > 0.0f) { velocity_.x *= (1.0f - kAttenuation); } // 急ブレーキ
		acceleration.x -= kAcceleration;
	}
	// ※ AとDの同時押し (pushA && pushD) の場合は、else ifで拾われないため acceleration.x は 0 のまま

	// 非入力時（A, Dともに押されていない、または A+D の同時押し）はX軸の移動減衰
	if (!inputX) {
		velocity_.x *= (1.0f - kAttenuation);
	}


	// ----------------------------------------------------
	// 加速度ベクトルの長さの制限（斜め入力時の調整）
	// ----------------------------------------------------
	// 加速度ベクトルの長さを取得
	float accelerationLength = Length(acceleration);

	// 加速度ベクトルの長さがkAccelerationを超えていたら正規化
	if (accelerationLength > kAcceleration) {
		acceleration = acceleration * (kAcceleration / accelerationLength);
	}

	// ----------------------------------------------------
	// 速度の更新とベクトルの長さの制限
	// ----------------------------------------------------

	// 加速/減速を速度に適用
	velocity_.x += acceleration.x;
	velocity_.z += acceleration.z;

	// 速度ベクトルの長さを取得
	float velocityLength = Length(velocity_);

	// 速度ベクトルの長さが最大速度kLimitRunSpeedを超えていたら制限
	if (velocityLength > kLimitRunSpeed) {
		velocity_ = velocity_ * (kLimitRunSpeed / velocityLength);
	}
	//if (!onGround_) {
		//	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
		//		// ジャンプ初速
		//		velocity_ += Vector3{0.0f, kJumpAcceleration, 0.0f};
		//	}
		//} else {
			// 落下速度
		//velocity_ += Vector3{ 0.0f, -kGravityAcceleration, 0.0f };
		//// 落下速度制限
		//velocity_.y = (std::max)(velocity_.y, -kLimitFallSpeed);
	//}

#ifdef _DEBUG

	if (Input::GetInstance()->TriggerKey(DIK_UP)) {
		// ジャンプ初速
		velocity_ += Vector3{ 0.0f, kJumpAcceleration, 0.0f };
	}
	if (Input::GetInstance()->TriggerKey(DIK_DOWN)) {
		// ジャンプ初速
		velocity_ += Vector3{ 0.0f, -kJumpAcceleration, 0.0f };
	}

#endif // DEBUG

}

#if 0 // マップチップ衝突判定を無効化

void Player::MapCollisionCheck(CollisionMapinfo& info) {
	//MapCollisionCheckUp(info);
	//MapCollisionCheckDown(info);
	MapCollisionCheckRight(info);
	MapCollisionCheckLeft(info);
}

void Player::MapCollisionCheckUp(CollisionMapinfo& info) {
	// 上昇有り？
	if (info.moveAmount_.y <= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionNew =
		CalculateCornerPositions(worldTransform_.translation_ + info.moveAmount_);

	//MapChipType mapChipType;
	// 真上の当たり判定を行う
	bool hit = false;
	// 初期化を追加
	MapChipField::IndexSet indexSetLeftTop = MapChipField::IndexSet(), indexSetRightTop = MapChipField::IndexSet();

	// 複数のブロックをまとめてチェックする
	for (float y = 0.0f; y <= info.moveAmount_.y; y += mapChipField_->GetChipSize()) {
		Vector3 checkPosLeftTop = positionNew[static_cast<uint32_t>(Corner::kLeftTop)] - Vector3{ 0, y, 0 };
		Vector3 checkPosRightTop = positionNew[static_cast<uint32_t>(Corner::kRightTop)] - Vector3{ 0, y, 0 };

		indexSetLeftTop = mapChipField_->GetMapChipIndexSetByPosition(checkPosLeftTop);
		indexSetRightTop = mapChipField_->GetMapChipIndexSetByPosition(checkPosRightTop);

		if (mapChipField_->GetMapChipTypeByIndex(indexSetLeftTop.xIndex, indexSetLeftTop.yIndex) == MapChipType::kBlock ||
			mapChipField_->GetMapChipTypeByIndex(indexSetRightTop.xIndex, indexSetRightTop.yIndex) == MapChipType::kBlock) {
			hit = true;
			break;
		}
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		MapChipField::Rect rect;
		// 左上のブロックで衝突した場合
		if (mapChipField_->GetMapChipTypeByIndex(indexSetLeftTop.xIndex, indexSetLeftTop.yIndex) == MapChipType::kBlock) {
			rect = mapChipField_->GetRectByIndex(indexSetLeftTop.xIndex, indexSetLeftTop.yIndex);
		} else {
			// 右上のブロックで衝突した場合
			rect = mapChipField_->GetRectByIndex(indexSetRightTop.xIndex, indexSetRightTop.yIndex);
		}

		float limitAmount = rect.bottom - (worldTransform_.translation_.y + kHeight / 2.0f);
		info.moveAmount_.y = (std::min)(info.moveAmount_.y, limitAmount);
		// 天井に当たったことを記録する
		info.onCollisionCeiling_ = true;
	}
}

void Player::MapCollisionCheckDown(CollisionMapinfo& info) {
	// 下降あり？
	if (info.moveAmount_.y >= 0) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionNew =
		CalculateCornerPositions(worldTransform_.translation_ + info.moveAmount_);

	//MapChipType mapChipType;
	// 真下の当たり判定を行う
	bool hit = false;
	// 初期化を追加
	MapChipField::IndexSet indexSetLeftBottom = MapChipField::IndexSet(), indexSetRightBottom = MapChipField::IndexSet();

	// 複数のブロックをまとめてチェックする
	for (float y = 0.0f; y >= info.moveAmount_.y; y -= mapChipField_->GetChipSize()) {
		Vector3 checkPosLeftBottom = positionNew[static_cast<uint32_t>(Corner::kLeftBottom)] - Vector3{ 0, y, 0 };
		Vector3 checkPosRightBottom = positionNew[static_cast<uint32_t>(Corner::kRightBottom)] - Vector3{ 0, y, 0 };

		indexSetLeftBottom = mapChipField_->GetMapChipIndexSetByPosition(checkPosLeftBottom);
		indexSetRightBottom = mapChipField_->GetMapChipIndexSetByPosition(checkPosRightBottom);

		if (mapChipField_->GetMapChipTypeByIndex(indexSetLeftBottom.xIndex, indexSetLeftBottom.yIndex) == MapChipType::kBlock ||
			mapChipField_->GetMapChipTypeByIndex(indexSetRightBottom.xIndex, indexSetRightBottom.yIndex) == MapChipType::kBlock) {
			hit = true;
			break;
		}
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		// どちらのブロックに衝突したかによってRectを取得するブロックを変える
		MapChipField::Rect rect;
		if (mapChipField_->GetMapChipTypeByIndex(indexSetLeftBottom.xIndex, indexSetLeftBottom.yIndex) == MapChipType::kBlock) {
			rect = mapChipField_->GetRectByIndex(indexSetLeftBottom.xIndex, indexSetLeftBottom.yIndex);
		} else {
			// 右下のブロックで衝突した場合
			rect = mapChipField_->GetRectByIndex(indexSetRightBottom.xIndex, indexSetRightBottom.yIndex);
		}

		float limitAmount = rect.top - (worldTransform_.translation_.y - kHeight / 2.0f);
		info.moveAmount_.y = (std::max)(info.moveAmount_.y, limitAmount);
		// 地面に当たったことを記録する
		info.onCollisionGround_ = true;
	}
}

void Player::MapCollisionCheckRight(CollisionMapinfo& info) {
	// 右移動あり？
	if (info.moveAmount_.x <= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionNew =
		CalculateCornerPositions(worldTransform_.translation_ + info.moveAmount_);

	//MapChipType mapChipType;
	// 右側の当たり判定を行う
	bool hit = false;
	// 初期化を追加
	MapChipField::IndexSet indexSetRightBottom = MapChipField::IndexSet(), indexSetRightTop = MapChipField::IndexSet();

	// 複数のブロックをまとめてチェックする
	for (float x = 0.0f; x <= info.moveAmount_.x; x += mapChipField_->GetChipSize()) {
		Vector3 checkPosRightBottom = positionNew[static_cast<uint32_t>(Corner::kRightBottom)] - Vector3{ x, 0, 0 };
		Vector3 checkPosRightTop = positionNew[static_cast<uint32_t>(Corner::kRightTop)] - Vector3{ x, 0, 0 };

		indexSetRightBottom = mapChipField_->GetMapChipIndexSetByPosition(checkPosRightBottom);
		indexSetRightTop = mapChipField_->GetMapChipIndexSetByPosition(checkPosRightTop);

		if (mapChipField_->GetMapChipTypeByIndex(indexSetRightBottom.xIndex, indexSetRightBottom.yIndex) == MapChipType::kBlock ||
			mapChipField_->GetMapChipTypeByIndex(indexSetRightTop.xIndex, indexSetRightTop.yIndex) == MapChipType::kBlock) {
			hit = true;
			break;
		}
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		MapChipField::Rect rect;
		// 右下のブロックで衝突した場合
		if (mapChipField_->GetMapChipTypeByIndex(indexSetRightBottom.xIndex, indexSetRightBottom.yIndex) == MapChipType::kBlock) {
			rect = mapChipField_->GetRectByIndex(indexSetRightBottom.xIndex, indexSetRightBottom.yIndex);
		} else {
			// 右上のブロックで衝突した場合
			rect = mapChipField_->GetRectByIndex(indexSetRightTop.xIndex, indexSetRightTop.yIndex);
		}

		float limitAmount = rect.left - (worldTransform_.translation_.x + kWidth / 2.0f);

		// プレイヤーが完全にブロックの境界線に張り付くのを防ぐため、微小なオフセット(kBlankなど)を適用
		// limitAmount が負の値（左方向の排除量）であることを前提に、さらに左に微小移動させる
		limitAmount -= kBlank;

		info.moveAmount_.x = (std::min)(info.moveAmount_.x, limitAmount);
		// 壁に当たったことを記録する
		info.onCollisionWall_ = true;
	}
}

void Player::MapCollisionCheckLeft(CollisionMapinfo& info) {
	// 左移動あり？
	if (info.moveAmount_.x >= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionNew =
		CalculateCornerPositions(worldTransform_.translation_ + info.moveAmount_);

	//MapChipType mapChipType;
	// 左側の当たり判定を行う
	bool hit = false;
	// 初期化を追加
	MapChipField::IndexSet indexSetLeftBottom = MapChipField::IndexSet(), indexSetLeftTop = MapChipField::IndexSet();

	// 複数のブロックをまとめてチェックする
	for (float x = 0.0f; x >= info.moveAmount_.x; x -= mapChipField_->GetChipSize()) {
		Vector3 checkPosLeftBottom = positionNew[static_cast<uint32_t>(Corner::kLeftBottom)] - Vector3{ x, 0, 0 };
		Vector3 checkPosLeftTop = positionNew[static_cast<uint32_t>(Corner::kLeftTop)] - Vector3{ x, 0, 0 };

		indexSetLeftBottom = mapChipField_->GetMapChipIndexSetByPosition(checkPosLeftBottom);
		indexSetLeftTop = mapChipField_->GetMapChipIndexSetByPosition(checkPosLeftTop);

		if (mapChipField_->GetMapChipTypeByIndex(indexSetLeftBottom.xIndex, indexSetLeftBottom.yIndex) == MapChipType::kBlock ||
			mapChipField_->GetMapChipTypeByIndex(indexSetLeftTop.xIndex, indexSetLeftTop.yIndex) == MapChipType::kBlock) {
			hit = true;
			break;
		}
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		MapChipField::Rect rect;
		// 左下のブロックで衝突した場合
		if (mapChipField_->GetMapChipTypeByIndex(indexSetLeftBottom.xIndex, indexSetLeftBottom.yIndex) == MapChipType::kBlock) {
			rect = mapChipField_->GetRectByIndex(indexSetLeftBottom.xIndex, indexSetLeftBottom.yIndex);
		} else {
			// 左上のブロックで衝突した場合
			rect = mapChipField_->GetRectByIndex(indexSetLeftTop.xIndex, indexSetLeftTop.yIndex);
		}

		float limitAmount = rect.right - (worldTransform_.translation_.x - kWidth / 2.0f);

		limitAmount += kBlank;

		info.moveAmount_.x = (std::max)(info.moveAmount_.x, limitAmount);
		// 壁に当たったことを記録する
		info.onCollisionWall_ = true;
	}
}

#endif // #if 0: マップチップ衝突判定の無効化を終了

Vector3 Player::CornerPosition(Vector3 center, Corner corner) {
	Vector3 offsetTable[static_cast<uint32_t>(Corner::kNumCorner)] = {
		Vector3{+kWidth / 2.0f, -kHeight / 2.0f, 0}, // kRightBottom
		Vector3{-kWidth / 2.0f, -kHeight / 2.0f, 0}, // kLeftBottom
		Vector3{+kWidth / 2.0f, +kHeight / 2.0f, 0}, // kRightTop
		Vector3{-kWidth / 2.0f, +kHeight / 2.0f, 0}, // kLeftTop
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

std::array<Vector3, static_cast<uint32_t>(Player::Corner::kNumCorner)> Player::CalculateCornerPositions(const Vector3& basePosition) {
	std::array<Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positions;

	for (uint32_t i = 0; i < positions.size(); ++i) {
		// CornerPositionはbasePositionと、各コーナーの種類を受け取る
		positions[i] = CornerPosition(basePosition, static_cast<Corner>(i));
	}
	return positions;
}

void Player::ApplyCollisionResult(const CollisionMapinfo& info) {
	// 移動
	worldTransform_.translation_ += info.moveAmount_;
}

void Player::OnCollisionCeiling(const CollisionMapinfo& info) {
	// 天井に当たった？
	if (info.onCollisionCeiling_) {
		velocity_.y = 0.0f;
	}
}

void Player::OnCollisionWall(const CollisionMapinfo& info) {
	// 壁に当たった？
	if (info.onCollisionWall_) {
		// 速度をゼロにする

		// X軸の速度リセット判定:
		// 元の速度の大きさと比較し、衝突後の移動量が十分に小さい場合のみ速度をリセットする
		// 衝突によりX軸の移動量が大きく制限された場合
		if (std::abs(info.moveAmount_.x) < std::abs(velocity_.x) * 0.5f) { // 例: 50%以上の減衰があった場合
			velocity_.x = 0.0f;
		}

		// Z軸の速度リセット判定:
		// 衝突によりZ軸の移動量が大きく制限された場合
		if (std::abs(info.moveAmount_.z) < std::abs(velocity_.z) * 0.5f) { // 例: 50%以上の減衰があった場合
			velocity_.z = 0.0f;
		}

		// Y軸
	}
}

void Player::OnGroundSwitch(const CollisionMapinfo& info) {
	// 自キャラが接地状態？
	if (onGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			onGround_ = false;
		} else {
			//// 移動後の4つの角の座標
			//std::array<Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionNew =
			//	CalculateCornerPositions(worldTransform_.translation_ + info.moveAmount_);

			////MapChipType mapChipType;
			//// 真下の当たり判定を行う
			//bool hit = false;
			//MapChipField::IndexSet indexSetLeftBottom = MapChipField::IndexSet(), indexSetRightBottom = MapChipField::IndexSet();

			//// 複数のブロックをまとめてチェックする
			//for (float y = 0.0f; y >= info.moveAmount_.y; y -= mapChipField_->GetChipSize()) {
			//	Vector3 checkPosLeftBottom = positionNew[static_cast<uint32_t>(Corner::kLeftBottom)] - Vector3{0, y, 0};
			//	Vector3 checkPosRightBottom = positionNew[static_cast<uint32_t>(Corner::kRightBottom)] - Vector3{0, y, 0};

			//	indexSetLeftBottom = mapChipField_->GetMapChipIndexSetByPosition(checkPosLeftBottom);
			//	indexSetRightBottom = mapChipField_->GetMapChipIndexSetByPosition(checkPosRightBottom);

			//	if (mapChipField_->GetMapChipTypeByIndex(indexSetLeftBottom.xIndex, indexSetLeftBottom.yIndex) == MapChipType::kBlock ||
			//		mapChipField_->GetMapChipTypeByIndex(indexSetRightBottom.xIndex, indexSetRightBottom.yIndex) == MapChipType::kBlock) {
			//		hit = true;
			//		break;
			//	}
			//}

			//// ブロックにヒット？
			//if (!hit) {
			onGround_ = false; // 接地状態を解除
			/*}*/
		}
	} else {
		if (info.onCollisionGround_) {
			// 着地状態に切り替える（落下を止める）
			onGround_ = true;
			// 着地時にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.z *= (1.0f - kAttenuationLanding);
			// Y速度をゼロにする
			velocity_.y = 0.0f;
		}
	}
}