#pragma once

#include <cmath>

struct Vec2 {
    float x;
    float y;

    Vec2 operator+(const Vec2 &other) const { return {x + other.x, y + other.y}; }

    Vec2 operator*(float scalar) const { return {x * scalar, y * scalar}; }

    Vec2 operator+=(const Vec2 &other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2 operator-(const Vec2& other) const {
        return {
            x - other.x,
            y - other.y
        };
    }

    Vec2 operator/(float scalar) const {
        return { x / scalar, y / scalar };
    };

    float length() const {
        return std::sqrt(x * x + y * y);
    }

    Vec2 normalized() const {
        float len = length();

        if (len == 0.0f) {
            return {0.0f, 0.0f};
        }

        return *this / len;
    }
    
    float dot(const Vec2& other) const {
        return x * other.x + y * other.y;
    }
};
