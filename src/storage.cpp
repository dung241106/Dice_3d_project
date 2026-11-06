#include "storage.hpp"
#include <fstream>
#include <iomanip>

namespace dice
{
bool exportOBJ(const std::filesystem::path& file, const Mesh& mesh, const Mat4& model)
{
    std::error_code error;
    std::filesystem::create_directories(file.parent_path(), error);
    if (error)
    {
        return false;
    }
    std::ofstream out(file);
    if (!out)
    {
        return false;
    }
    out << std::setprecision(10);
    out << "# Dice3D Education Edition: body mesh, pips are separate procedural discs\no "
           "dice_body\n";
    for (auto v : mesh.vertices)
    {
        auto p = model.point(v.position);
        out << "v " << p.x << ' ' << p.y << ' ' << p.z << '\n';
    }
    for (auto v : mesh.vertices)
    {
        auto n = model.normal(v.normal);
        out << "vn " << n.x << ' ' << n.y << ' ' << n.z << '\n';
    }
    Vec3 x{model.m[0], model.m[1], model.m[2]}, y{model.m[4], model.m[5], model.m[6]},
        z{model.m[8], model.m[9], model.m[10]};
    bool reflected = x.dot(y.cross(z)) < 0;
    for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
    {
        out << "f";
        for (int j : {0, reflected ? 2 : 1, reflected ? 1 : 2})
        {
            unsigned k = mesh.indices[i + static_cast<std::size_t>(j)] + 1;
            out << ' ' << k << "//" << k;
        }
        out << '\n';
    }
    out.close();
    return bool(out);
}

bool exportTransforms(const std::filesystem::path& file, const Lab& lab)
{
    std::error_code error;
    std::filesystem::create_directories(file.parent_path(), error);
    if (error)
    {
        return false;
    }
    std::ofstream out(file);
    if (!out)
    {
        return false;
    }
    out << "step,kind,axis,amount,px,py,pz,kx,ky,kz\n" << std::setprecision(12);
    for (std::size_t i = 0; i < lab.cursor; i++)
    {
        const auto& s = lab.steps[i];
        out << i + 1 << ',' << static_cast<int>(s.kind) << ',' << s.axis << ',' << s.amount << ','
            << s.pivot.x << ',' << s.pivot.y << ',' << s.pivot.z << ',' << s.arbitraryAxis.x << ','
            << s.arbitraryAxis.y << ',' << s.arbitraryAxis.z << '\n';
    }
    out.close();
    return bool(out);
}
} // namespace dice
