#ifndef SHARED_CONST_HH
#define SHARED_CONST_HH 1
#pragma once

#include "core/constexpr.hh"

constexpr static unsigned int CHUNK_SIZE = 16;
constexpr static unsigned int CHUNK_AREA = CHUNK_SIZE * CHUNK_SIZE;
constexpr static unsigned int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
constexpr static unsigned int CHUNK_BITSHIFT = cxpr::log2(CHUNK_SIZE);

template<typename T>
constexpr static glm::vec<3, T> DIR_NORTH = glm::vec<3, T>(0, 0, +1);
template<typename T>
constexpr static glm::vec<3, T> DIR_SOUTH = glm::vec<3, T>(0, 0, -1);
template<typename T>
constexpr static glm::vec<3, T> DIR_EAST = glm::vec<3, T>(-1, 0, 0);
template<typename T>
constexpr static glm::vec<3, T> DIR_WEST = glm::vec<3, T>(+1, 0, 0);
template<typename T>
constexpr static glm::vec<3, T> DIR_DOWN = glm::vec<3, T>(0, -1, 0);
template<typename T>
constexpr static glm::vec<3, T> DIR_UP = glm::vec<3, T>(0, +1, 0);

template<typename T>
constexpr static glm::vec<3, T> DIR_FORWARD = glm::vec<3, T>(0, 0, +1);
template<typename T>
constexpr static glm::vec<3, T> DIR_BACK = glm::vec<3, T>(0, 0, -1);
template<typename T>
constexpr static glm::vec<3, T> DIR_LEFT = glm::vec<3, T>(-1, 0, 0);
template<typename T>
constexpr static glm::vec<3, T> DIR_RIGHT = glm::vec<3, T>(+1, 0, 0);

template<typename T>
constexpr static glm::vec<3, T> UNIT_X = glm::vec<3, T>(1, 0, 0);
template<typename T>
constexpr static glm::vec<3, T> UNIT_Y = glm::vec<3, T>(0, 1, 0);
template<typename T>
constexpr static glm::vec<3, T> UNIT_Z = glm::vec<3, T>(0, 0, 1);

template<typename T>
constexpr static glm::vec<2, T> ZERO_VEC2 = glm::vec<2, T>(0, 0);

template<typename T>
constexpr static glm::vec<3, T> ZERO_VEC3 = glm::vec<3, T>(0, 0, 0);

#endif /* SHARED_CONST_HH */
