#pragma once
#include "lab.hpp"

namespace dice
{
bool exportOBJ(const std::filesystem::path& file, const Mesh& mesh, const Mat4& model = Mat4{});
bool exportTransforms(const std::filesystem::path& file, const Lab& lab);
} // namespace dice
