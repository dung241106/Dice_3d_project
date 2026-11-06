#include "dice.hpp"
#include "lab.hpp"
#include "view.hpp"
#include "puzzle.hpp"
#include "throw_sim.hpp"
#include "storage.hpp"
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>

namespace dice
{
int runTests()
{
    int checks = 0;
    auto require = [&](bool ok, const char* label)
    {
        ++checks;
        if (!ok)
        {
            throw std::runtime_error(label);
        }
    };
    auto near = [](Vec3 a, Vec3 b)
    {
        return (a - b).length() < 1e-8;
    };
    try
    {
        for (auto f : faces())
        {
            require(near(f.u.cross(f.v), f.normal), "Face basis winding");
            require(pipCenters(f.value).size() == static_cast<std::size_t>(f.value), "Pip count");
            for (auto g : faces())
            {
                if (f.normal.dot(g.normal) < -.99)
                {
                    require(f.value + g.value == 7, "Opposite faces sum to seven");
                }
            }
            for (int j = 0; j < 24; j++)
            {
                auto q = faceUp(f.value, j * pi / 12);
                require(topFace(q) == f.value, "True upper face");
                require(std::abs(supportHeight(q) - half) < 1e-9, "Resting support");
            }
        }
        auto mesh = roundedCube();
        require(mesh.vertices.size() == 2646 && mesh.indices.size() == 14400, "Rounded mesh count");
        auto cube = basicCube();
        require(cube.vertices.size() == 24 && cube.indices.size() == 36,
                "24 normal-split vertices for 8 cube corners");
        for (auto v : mesh.vertices)
        {
            require(std::abs(v.normal.length() - 1) < 1e-9, "Unit normal");
        }
        for (std::size_t i = 0; i < mesh.indices.size(); i += 3)
        {
            auto a = mesh.vertices[mesh.indices[i]], b = mesh.vertices[mesh.indices[i + 1]],
                 c = mesh.vertices[mesh.indices[i + 2]];
            require((b.position - a.position)
                            .cross(c.position - a.position)
                            .dot(a.normal + b.normal + c.normal) > 0,
                    "Outward triangle");
        }
        std::mt19937 rng(12345);
        std::uniform_real_distribution<double> real(-1, 1);
        for (int i = 0; i < 100; i++)
        {
            auto q = Quat::axisAngle({real(rng), real(rng), real(rng)}, real(rng) * pi);
            auto r = q.matrix();
            Mat4 m;
            m.m = r;
            Vec3 p{real(rng), real(rng), real(rng)};
            require(near(m.point(p), q.rotate(p)), "Quaternion equals column-major matrix");
            for (auto v : mesh.vertices)
            {
                require(q.rotate(v.position).y + supportHeight(q) > -1e-9, "Rounded support bound");
            }
            auto a = Mat4::translation(p) * m * Mat4::scale({.5 + std::abs(real(rng)), 1.2, -.8});
            Mat4 inverse;
            require(a.inverse(inverse) && near((inverse * a).point(p), p),
                    "Inverse affine composition");
            Vec3 t = a.point({0, 1, 0}) - a.point({});
            require(std::abs(a.normal({1, 0, 0}).dot(t)) < 1e-8, "Inverse-transpose normal");
        }
        Mat4 singular = Mat4::scale({0, 1, 1}), inverse;
        require(!singular.inverse(inverse), "Zero-scale has no inverse");
        Mat4 view = lookAt({0, 0, 10}, {0, 0, 0}, {0, 1, 0});
        require(near(view.point({}), {0, 0, -10}), "OpenGL eye looks down -Z");
        auto pr = perspective(60, 2, .1, 80);
        auto center = project({}, Mat4{}, view, pr, 1000, 500);
        require(center.inside && std::abs(center.screen.x - 500) < 1e-8 &&
                    std::abs(center.screen.y - 250) < 1e-8,
                "Viewport center");
        auto nearP = project({1, 0, 0}, Mat4{}, view, pr, 1000, 500),
             farP = project({1, 0, -10}, Mat4{}, view, pr, 1000, 500);
        require(nearP.ndc.x > farP.ndc.x && std::abs(nearP.ndc.x / farP.ndc.x - 2) < 1e-8,
                "Perspective depth scaling");
        auto ortho = orthographic(-10, 10, -5, 5, .1, 80);
        nearP = project({1, 0, 0}, Mat4{}, view, ortho, 1000, 500);
        farP = project({1, 0, -10}, Mat4{}, view, ortho, 1000, 500);
        require(std::abs(nearP.ndc.x - farP.ndc.x) < 1e-8,
                "Orthographic preserves projected size with depth");
        require(near(cabinet().point({0, 0, 2}), {std::sqrt(.5), std::sqrt(.5), 2}),
                "Cabinet half-depth shear");
        auto behind = project({0, 0, 12}, Mat4{}, view, pr, 1000, 500);
        require(!behind.inside, "Point behind eye clipped");
        auto atEye = project({0, 0, 10}, Mat4{}, view, pr, 1000, 500);
        require(!atEye.valid, "No divide by zero at eye");
        TransformStep step{Transform::Rotate, 3, 60, {1, 0, 0}, {1, 1, 0}};
        require(near(step.matrix().point({1, 0, 0}), {1, 0, 0}), "Arbitrary pivot is fixed");
        require(near(step.matrix().point({2, 1, 0}), {2, 1, 0}),
                "Every point on arbitrary axis is fixed");
        std::set<int> poses;
        std::vector<Quat> orientations;
        for (int f = 1; f <= 6; f++)
        {
            for (int y = 0; y < 4; y++)
            {
                auto q = faceUp(f, y * pi / 2);
                poses.insert(poseKey(q));
                orientations.push_back(q);
            }
        }
        require(poses.size() == 24, "Exactly 24 cube orientations");
        int longest = 0;
        for (auto original : orientations)
        {
            for (auto b : orientations)
            {
                auto a = original;
                auto path = shortestTurns(a, b);
                longest = std::max(longest, int(path.size()));
                for (auto t : path)
                {
                    Vec3 axis = t.axis == 0   ? Vec3{1, 0, 0}
                                : t.axis == 1 ? Vec3{0, 1, 0}
                                              : Vec3{0, 0, 1};
                    a = (Quat::axisAngle(axis, t.sign * pi / 2) * a).unit();
                }
                require(poseKey(a) == poseKey(b), "BFS reaches target pose");
            }
        }
        require(longest <= 3, "Cube orientation graph diameter <=3");
        FacePuzzle puzzle(2026);
        int expected = puzzle.par;
        for (int i = 0; i < 3; i++)
        {
            puzzle.selected = i;
            auto path = shortestTurns(puzzle.rotations[i], puzzle.targets[i]);
            for (auto t : path)
            {
                require(puzzle.turn(t.axis, t.sign), "Valid move");
                require(!puzzle.turn(0, 1), "Animation overlap locked");
                puzzle.tick(.25);
            }
        }
        require(puzzle.solved && puzzle.moves == expected && puzzle.stars() == 3,
                "Optimal puzzle solution");
        auto testDir = std::filesystem::current_path() /
                       ("edu-export-test-" + std::to_string(std::random_device{}()));
        auto obj = testDir / "body.obj", csv = testDir / "transforms.csv";
        require(exportOBJ(obj, cube, Mat4::translation({2, 0, 0})), "OBJ export");
        int vertices = 0, normals = 0, triangles = 0;
        std::string line;
        std::ifstream input(obj);
        while (std::getline(input, line))
        {
            if (line.rfind("v ", 0) == 0)
            {
                ++vertices;
            }
            if (line.rfind("vn ", 0) == 0)
            {
                ++normals;
            }
            if (line.rfind("f ", 0) == 0)
            {
                ++triangles;
            }
        }
        require(vertices == 24 && normals == 24 && triangles == 12, "OBJ geometry counts");
        input.close();
        Lab exported;
        exported.tool = Transform::Rotate;
        exported.axis = 3;
        exported.pivotEnabled = true;
        exported.apply(1);
        require(exportTransforms(csv, exported), "Transform CSV export");
        std::ifstream chain(csv);
        std::getline(chain, line);
        require(line == "step,kind,axis,amount,px,py,pz,kx,ky,kz", "Transform CSV schema");
        std::getline(chain, line);
        require(line == "1,1,3,90,1,0,0,1,1,0", "Transform CSV stores pivot and arbitrary axis");
        chain.close();
        std::filesystem::remove(obj);
        std::filesystem::remove(csv);
        std::filesystem::remove(testDir);
        std::cout << "PASS: " << checks << " geometry / projection / BFS checks\n";
        if (testLab() != 0)
        {
            return 1;
        }
        return testThrows();
    }
    catch (const std::exception& e)
    {
        std::cerr << "FAIL after " << checks << ": " << e.what() << '\n';
        return 1;
    }
}
} // namespace dice
