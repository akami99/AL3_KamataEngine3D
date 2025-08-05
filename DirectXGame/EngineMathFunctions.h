#pragma once

#ifndef ENGINE_MATH_FUNCTIONS_H
#define ENGINE_MATH_FUNCTIONS_H

#include "EngineMath.h" // Vector3, Matrix4x4などの構造体定義を含む
#include "KamataEngine.h" // Sphere, Planeなどの構造体定義を含む

// 基本的なベクトル演算

/// <summary>
/// ベクトルの加算
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>ベクトルの和</returns>
KamataEngine::Vector3 Add(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

/// <summary>
/// ベクトルの減算
/// </summary>
/// <param name="v1">引かれるベクトル</param>
/// <param name="v2">引くベクトル</param>
/// <returns>ベクトルの差</returns>
KamataEngine::Vector3 Subtract(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

/// <summary>
/// ベクトルの積（要素ごとの積）
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>要素ごとの積</returns>
KamataEngine::Vector3 Multiply(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

/// <summary>
/// 内積
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>内積</returns>
float Dot(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

/// <summary>
/// クロス積
/// </summary>
/// <param name="v1">ベクトル1</param>
/// <param name="v2">ベクトル2</param>
/// <returns>クロス積</returns>
KamataEngine::Vector3 Cross(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2);

/// <summary>
/// 長さ（ノルム）
/// </summary>
/// <param name="v">ベクトル</param>
/// <returns>長さ（ノルム）</returns>
float Length(const KamataEngine::Vector3& v);

// 線形補間

/// <summary>
/// スムーズステップ関数
/// </summary>
/// <param name="t">補間係数</param>
/// <returns>スムーズステップ</returns>
float Smoothstep(float t);

/// <summary>
/// Lerp関数
/// </summary>
/// <param name="v1">始点</param>
/// <param name="v2">終点</param>
/// <param name="t">補間係数</param>
/// <returns>Lerp</returns>
KamataEngine::Vector3 Lerp(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2, float t);

/// <summary>
/// float型のLerp関数
/// </summary>
/// <param name="v1">始点</param>
/// <param name="v2">終点</param>
/// <param name="t">補間係数</param>
/// <returns>Lerp</returns>
float Lerp(float v1, float v2, float t);

// tを0から1に変換するイーズアウト関数
float EaseOutFloat(float t);

// イーズアウトを適用したLerp関数
float EaseOutLerpFloat(float v1, float v2, float t);

// tを0から1に変換するイーズイン関数
float EaseInFloat(float t);

// イーズインを適用したLerp関数
float EaseInLerpFloat(float v1, float v2, float t);

/// <summary>
/// イーズインアウトを適用したLerp関数
/// </summary>
/// <param name="v1">始点</param>
/// <param name="v2">終点</param>
/// <param name="t">補間係数</param>
/// <returns>イーズインアウトを適用したLerp</returns>
KamataEngine::Vector3 EaseInOutLerp(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2, float t);

/// <summary>
/// イーズインアウトを適用したfloat型のLerp関数
/// </summary>
/// <param name="v1">始点</param>
/// <param name="v2">終点</param>
/// <param name="t">補間係数</param>
/// <returns>イーズインアウトを適用したfloat型のLerp</returns>
float EaseInOutLerpFloat(float v1, float v2, float t);

/// <summary>
/// 正規化
/// </summary>
/// <param name="v">ベクトル</param>
/// <returns>正規化されたベクトル</returns>
KamataEngine::Vector3 Normalize(const KamataEngine::Vector3& v);

/// <summary>
/// 垂直なベクトルを求める関数
/// </summary>
/// <param name="vector">基となるベクトル</param>
/// <returns>垂直なベクトル</returns>
KamataEngine::Vector3 Perpendicular(const KamataEngine::Vector3& vector);

/// <summary>
/// 反射ベクトルを求める関数
/// </summary>
/// <param name="input">入射ベクトル</param>
/// <param name="normal">法線(正規化していないもの)</param>
/// <returns>反射ベクトル</returns>
KamataEngine::Vector3 Reflect(const KamataEngine::Vector3& input, const KamataEngine::Vector3& normal);

/// <summary>
/// 球との衝突判定を行う関数
/// </summary>
/// <param name="sphere">球</param>
/// <param name="plane">平面</param>
/// <returns>衝突判定</returns>
bool IsCollision(const Sphere& sphere, const Plane& plane);

/// <summary>
/// AABBとAABBの衝突判定関数
/// </summary>
/// <param name="aabb1">AABB</param>
/// <param name="aabb2">AABB</param>
/// <returns>衝突判定</returns>
bool IsCollision(const AABB& aabb1, const AABB& aabb2);

/// <summary>
/// ベクトルを法線方向に投影する関数
/// </summary>
/// <param name="vector">ベクトル</param>
/// <param name="normal">法線(正規化していないもの)</param>
/// <returns>投影ベクトル</returns>
KamataEngine::Vector3 Project(const KamataEngine::Vector3& vector, const KamataEngine::Vector3& normal);

// 基本的な行列演算

/// <summary>
/// 4x4行列の加算
/// </summary>
/// <param name="v1">行列1</param>
/// <param name="v2">行列2</param>
/// <returns>行列の和</returns>
KamataEngine::Matrix4x4 Add(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);

/// <summary>
/// 4x4行列の減算
/// </summary>
/// <param name="v1">引かれる4x4行列</param>
/// <param name="v2">引く4x4行列</param>
/// <returns>4x4行列の差</returns>
KamataEngine::Matrix4x4 Subtract(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);

/// <summary>
/// 4x4行列の積
/// </summary>
/// <param name="m1">掛ける行列1</param>
/// <param name="m2">掛ける行列2</param>
/// <returns>行列の積</returns>
KamataEngine::Matrix4x4 Multiply(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);

/// <summary>
/// 逆行列
/// </summary>
/// <param name="m">逆行列に変換する行列</param>
/// <returns>逆行列</returns>
KamataEngine::Matrix4x4 Inverse(const KamataEngine::Matrix4x4& m);

// スカラー倍

/// <summary>
/// スカラーとベクトルの積
/// </summary>
/// <param name="scalar">掛けるスカラー</param>
/// <param name="vector">掛けるベクトル</param>
/// <returns>ベクトルの積</returns>
KamataEngine::Vector3 Multiply(const float& scalar, const KamataEngine::Vector3& vector);

/// <summary>
/// スカラーと4x4行列の積
/// </summary>
/// <param name="scalar">掛けるスカラー</param>
/// <param name="matrix">掛ける4x4行列</param>
/// <returns>4x4行列の積</returns>
KamataEngine::Matrix4x4 Multiply(const float& scalar, const KamataEngine::Matrix4x4& matrix); // ここはKamataEngine::Matrix4x4にしておきました

// 変換

/// <summary>
/// 座標変換
/// </summary>
/// <param name="vector">変換したいベクトル</param>
/// <param name="matrix">変換させる行列</param>
/// <returns>変換させた座標</returns>
KamataEngine::Vector3 TransformPoint(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& matrix);

/// <summary>
/// ベクトル変換
/// </summary>
/// <param name="vector">変換したいベクトル</param>
/// <param name="matrix">変換させる行列</param>
/// <returns>変換させたベクトル</returns>
KamataEngine::Vector3 TransformVector(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& matrix);

/// <summary>
/// 球面をデカルト座標に変換
/// </summary>
/// <param name="radius">半径</param>
/// <param name="lat">緯度</param>
/// <param name="lon">経度</param>
/// <returns>デカルト座標</returns>
KamataEngine::Vector3 SphericalToCartesian(float radius, float lat, float lon);

// 演算子オーバーロード（インライン関数としてヘッダーに定義）
// 演算子の実装は、上記の関数を呼び出すだけなので、通常ヘッダーファイルに直接記述します。
inline KamataEngine::Vector3 operator+(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2) {
	return Add(v1, v2);
}
inline KamataEngine::Vector3 operator-(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2) {
	return Subtract(v1, v2);
}
inline KamataEngine::Vector3 operator*(float s, const KamataEngine::Vector3& v) {
	return Multiply(s, v);
}
inline KamataEngine::Vector3 operator*(const KamataEngine::Vector3& v, float s) {
	return s * v;
}
inline KamataEngine::Vector3 operator*(const KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2) {
	return Multiply(v1, v2);
}
inline KamataEngine::Vector3 operator/(const KamataEngine::Vector3& v, float s) {
	return { Multiply(1.0f / s, v) };
}
inline KamataEngine::Vector3 operator+=(KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2) {
	v1 = Add(v1, v2);
	return v1;
}
inline KamataEngine::Vector3 operator-=(KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2) {
	v1 = Subtract(v1, v2);
	return v1;
}
inline KamataEngine::Vector3 operator*=(KamataEngine::Vector3& v, const float& s) {
	v = Multiply(s, v);
	return v;
}
inline KamataEngine::Vector3 operator*=(KamataEngine::Vector3& v1, const KamataEngine::Vector3& v2) {
	v1 = Multiply(v1, v2);
	return v1;
}
inline KamataEngine::Matrix4x4 operator+(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2) {
	return Add(m1, m2);
}
inline KamataEngine::Matrix4x4 operator-(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2) {
	return Subtract(m1, m2);
}
inline KamataEngine::Matrix4x4 operator*(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2) {
	return Multiply(m1, m2);
}
inline KamataEngine::Matrix4x4 operator+=(KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2) {
	m1 = Add(m1, m2);
	return m1;
}
inline KamataEngine::Matrix4x4 operator-=(KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2) {
	m1 = Subtract(m1, m2);
	return m1;
}
inline KamataEngine::Matrix4x4 operator*=(KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2) {
	m1 = Multiply(m1, m2);
	return m1;
}

#endif // ENGINE_MATH_FUNCTIONS_H
