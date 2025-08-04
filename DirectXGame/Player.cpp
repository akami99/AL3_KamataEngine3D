#define NOMIMAX

#include "Player.h"
#include "EngineMathFunctions.h"
#include "WorldTransform.h"
#include "MapChipField.h"
#include <cassert>
#include <numbers>
#include <algorithm>

using namespace KamataEngine;

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	// NULLポインタチェック
	assert(model);

	// 引数の内容をメンバ変数に記録
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

void Player::Update() {
	// 1.移動入力
	Move();

	// 2.移動量を加味して衝突判定する
	// 衝突情報を初期化
	CollisionMapinfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.moveAmount_ = velocity_;

	// マップ衝突チェック
	MapCollisionCheck(collisionMapInfo);

	// 3.判定結果を反映して移動させる
	ApplyCollisionResult(collisionMapInfo);

	// 4.天井に接触している場合の処理
	OnCollisionCeiling(collisionMapInfo);

	// 5.壁に接触している場合の処理(後で)

	// 6.接地状態の切り替え
	// 着地フラグ
	bool landing = false;

	// 地面との当たり判定
	// 下降中？
	if (velocity_.y < 0) {
		// Y座標が地面以下になったら着地
		if (worldTransform_.translation_.y <= 1.0f) {
			landing = true;
		}
	}

	// 接地判定
	if (onGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			onGround_ = false;
		}
	} else {
		// 着地
		if (landing) {
			// めり込み排斥
			worldTransform_.translation_.y = 1.0f;
			// 摩擦で横方向速度が減衰する
			velocity_.x *= (1.0f - kAttenuation);
			// 下方向に速度をリセット
			velocity_.y = 0.0f;
			// 接地状態に移行
			onGround_ = true;
		}
	}

	// 7.旋回制御
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;

		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = {
			std::numbers::pi_v<float> / 2.0f, // 右
			std::numbers::pi_v<float> *3.0f / 2.0f, // 左
		};
		// 状況に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		// 自キャラの角度を設定する
		worldTransform_.rotation_.y = EaseInOutLerpFloat(destinationRotationY, turnFirstRotationY_, turnTimer_ / kTimeTurn);
	}

	// 8.行列計算
	UpdateWorldTransform(worldTransform_);
}

void Player::Draw() {
	// 3Dモデルを描画
	model_->Draw(worldTransform_, *camera_);
}

void Player::Move() {
	// 移動入力
	// 接地状態
	if (onGround_) {
		// 左右移動操作
		if (Input::GetInstance()->PushKey(DIK_RIGHT) ||
			Input::GetInstance()->PushKey(DIK_LEFT)) {

			// 左右加速
			Vector3 acceleration = {};
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
				// 左入力中の右入力
				if (velocity_.x < 0.0f) {
					// 速度と逆方向中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x += kAcceleration;

				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
				// 右入力中の左入力
				if (velocity_.x > 0.0f) {
					// 速度と逆方向中は急ブレーキ
					velocity_.x *= (1.0f - kAttenuation);
				}

				acceleration.x -= kAcceleration;

				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			}
			// 加速/減速
			velocity_ += acceleration;

			// 最大速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		} else {
			// 非入力時は移動減衰を書ける
			velocity_.x *= (1.0f - kAttenuation);
		}
		if (Input::GetInstance()->PushKey(DIK_UP)) {
			// ジャンプ初速
			velocity_ += Vector3(0.0f, kJumpAcceleration, 0.0f);
		}
		// 空中
	} else {
		// 落下速度
		velocity_ += Vector3(0.0f, -kGravityAcceleration, 0.0f);
		// 落下速度制限
		velocity_.y = (std::max)(velocity_.y, -kLimitFallSpeed);
	}
	// 移動
	worldTransform_.translation_ += velocity_;
}

void Player::MapCollisionCheck(CollisionMapinfo& info) {
	MapCollisionCheckUp(info);
	/*MapCollisionCheckDown(info);
	MapCollisionCheckRight(info);
	MapCollisionCheckLeft(info);*/
}

void Player::MapCollisionCheckUp(CollisionMapinfo& info) {
	// 上昇有り？
	if (info.moveAmount_.y <= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, static_cast<uint32_t>(Corner::kNumCorner)> positionNew =
		CalculateCornerPositions(worldTransform_.translation_ + info.moveAmount_);

	MapChipType mapChipType;
	// 真上の当たり判定を行う
	bool hit = false;
	// 左上点の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[static_cast<uint32_t>(Corner::kLeftTop)]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[static_cast<uint32_t>(Corner::kRightTop)]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[static_cast<uint32_t>(Corner::kLeftTop)]);
		// めり込み先ブロックの範囲矩形
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		
		float blueArrow = rect.bottom - worldTransform_.translation_.y; // worldTransform_.translation_.y は移動前自キャラの中心Y座標
		float playerHalfHeight = 2.0f / 2.0f; // プレイヤーの高さの半分 (適宜修正)
		float greenArrow = playerHalfHeight + kBlank;
		float desiredYMoveAmount = blueArrow - greenArrow;

		info.moveAmount_.y = (std::max)(0.0f, desiredYMoveAmount);
		// 天井に当たったことを記録する
		info.onCollisionCeiling_ = true;
	}
}

//void Player::MapCollisionCheckDown(CollisionMapinfo& info) {
//
//}
//
//void Player::MapCollisionCheckRight(CollisionMapinfo& info) {
//
//}
//
//void Player::MapCollisionCheckLeft(CollisionMapinfo& info) {
//
//}

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
