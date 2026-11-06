#include "throw_sim.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>

namespace dice
{
double supportExtent(Quat q, Vec3 direction)
{
    Vec3 a = Quat{q.w, -q.x, -q.y, -q.z}.unit().rotate(direction.unit());
    return (half - bevel) * (std::abs(a.x) + std::abs(a.y) + std::abs(a.z)) + bevel;
}

Vec3 supportPoint(Quat q, Vec3 direction)
{
    Vec3 a = Quat{q.w, -q.x, -q.y, -q.z}.unit().rotate(direction.unit());
    auto sign = [](double v)
    {
        return std::abs(v) < 1e-8 ? 0.0 : v > 0 ? 1.0 : -1.0;
    };
    return q.rotate(Vec3{sign(a.x), sign(a.y), sign(a.z)} * (half - bevel) + a * bevel);
}

ThrowSimulation::ThrowSimulation(std::uint32_t seed) : rng_(seed)
{
    setCount(3);
}

bool ThrowSimulation::setCount(int value)
{
    if (rolling || value < 1 || value > 3)
    {
        return false;
    }
    count = value;
    for (int i = 0; i < 3; i++)
    {
        bodies[i] = {};
        bodies[i].die = {{(i - (count - 1) * 0.5) * 2.4, half, 0}, faceUp(i + 1, 0.18), i + 1};
    }
    return true;
}

bool ThrowSimulation::launch(double power, Vec3 direction)
{
    if (rolling || !std::isfinite(power) || power < 0 || power > 1 || history.size() >= 100000 ||
        (!history.empty() && history.back().id >= 100000))
    {
        return false;
    }
    std::uniform_real_distribution<double> unit(-1, 1);
    lastPower = power;
    elapsed = 0;
    tableHits = 0;
    wallHits = 0;
    pairHits = 0;
    paused = false;
    rolling = true;
    for (int i = 0; i < count; i++)
    {
        auto& b = bodies[i];
        b.sleeping = false;
        b.settling = false;
        b.quiet = 0;
        b.velocity = {unit(rng_) * (2.0 + 6 * power), 3 + 4.5 * power,
                      unit(rng_) * (1.5 + 4 * power)};
        if (direction.length() > 0.01)
        {
            auto aim = direction.unit();
            b.velocity.x = aim.x * (2 + 6 * power) + b.velocity.x * 0.12;
            b.velocity.z = aim.z * (2 + 6 * power) + b.velocity.z * 0.12;
        }
        b.angular = {unit(rng_) * (5 + 13 * power), unit(rng_) * (5 + 13 * power),
                     unit(rng_) * (5 + 13 * power)};
        b.die.position.y = supportHeight(b.die.rotation) + 0.01;
    }
    return true;
}

void ThrowSimulation::contact(ThrowBody& b, Vec3 n, double penetration, bool table)
{
    if (penetration < 0)
    {
        return;
    }
    b.die.position = b.die.position + n * (penetration + 1e-6);
    Vec3 r = supportPoint(b.die.rotation, n * (-1));
    Vec3 v = b.velocity + b.angular.cross(r);
    double vn = v.dot(n);
    if (vn >= 0)
    {
        return;
    }
    constexpr double invInertia =
        6.0 / (4 * half * half); // Unit mass, isotropic cube approximation.
    double restitution = std::abs(vn) < 0.65 ? 0 : table ? 0.38 : 0.62;
    double impulse = -(1 + restitution) * vn / (1 + invInertia * r.cross(n).dot(r.cross(n)));
    Vec3 j = n * impulse;
    b.velocity = b.velocity + j;
    b.angular = b.angular + r.cross(j) * invInertia;
    Vec3 tangential = v - n * vn;
    double speed = tangential.length();
    if (speed > 1e-8)
    {
        Vec3 tangent = tangential * (1 / speed);
        double friction = std::min(
            0.45 * impulse, speed / (1 + invInertia * r.cross(tangent).dot(r.cross(tangent))));
        Vec3 jt = tangent * (-friction);
        b.velocity = b.velocity + jt;
        b.angular = b.angular + r.cross(jt) * invInertia;
    }
    if (-vn > 0.65)
    {
        if (table)
        {
            tableHits++;
            tableImpact = std::max(tableImpact, -vn);
        }
        else
        {
            wallHits++;
            wallImpact = std::max(wallImpact, -vn);
        }
    }
}

void ThrowSimulation::constrain(ThrowBody& b)
{
    double sx = supportExtent(b.die.rotation, {1, 0, 0}),
           sz = supportExtent(b.die.rotation, {0, 0, 1});
    contact(b, {0, 1, 0}, supportHeight(b.die.rotation) - b.die.position.y, true);
    const double x = wallX, z = wallZ;
    contact(b, {-1, 0, 0}, b.die.position.x + sx - x, false);
    contact(b, {1, 0, 0}, -x - b.die.position.x + sx, false);
    contact(b, {0, 0, -1}, b.die.position.z + sz - z, false);
    contact(b, {0, 0, 1}, -z - b.die.position.z + sz, false);
}

void ThrowSimulation::settle(ThrowBody& b)
{
    b.settling = true;
    b.settleTime = 0;
    b.settleStart = b.die.rotation;
    int value = topFace(b.die.rotation);
    Vec3 n{};
    for (auto f : faces())
    {
        if (f.value == value)
        {
            n = b.die.rotation.rotate(f.normal);
        }
    }
    b.settleTarget = (Quat::fromTo(n, {0, 1, 0}) * b.die.rotation).unit();
    b.velocity = {};
    b.angular = {};
}

void ThrowSimulation::integrate(double dt)
{
    elapsed += dt;
    for (int i = 0; i < count; i++)
    {
        auto& b = bodies[i];
        if (b.sleeping)
        {
            continue;
        }
        if (b.settling)
        {
            b.settleTime += dt;
            double u = std::min(1.0, b.settleTime / 0.4);
            b.die.rotation = Quat::slerp(b.settleStart, b.settleTarget, u * u * (3 - 2 * u));
            b.die.position.y = supportHeight(b.die.rotation);
            constrain(b);
            if (u >= 1)
            {
                b.sleeping = true;
                b.die.rotation = b.settleTarget;
                b.die.value = topFace(b.die.rotation);
            }
            continue;
        }
        b.velocity.y -= 9.81 * dt;
        b.velocity = b.velocity * std::exp(-0.12 * dt);
        b.angular = b.angular * std::exp(-0.18 * dt);
        b.die.position = b.die.position + b.velocity * dt;
        double spin = b.angular.length();
        if (spin > 1e-8)
        {
            b.die.rotation = (Quat::axisAngle(b.angular, spin * dt) * b.die.rotation).unit();
        }
        constrain(b);
        bool ground = b.die.position.y - supportHeight(b.die.rotation) < 0.025;
        if (ground)
        {
            b.velocity.x *= std::exp(-3.0 * dt);
            b.velocity.z *= std::exp(-3.0 * dt);
            b.angular = b.angular * std::exp(-5.0 * dt);
        }
        bool quiet = ground && b.velocity.length() < 0.65 && b.angular.length() < 1.8;
        b.quiet = quiet ? b.quiet + dt : 0;
        if (b.quiet > 0.22 || (elapsed > 7.5 && ground) || elapsed > 9)
        {
            settle(b);
        }
    }
    // Inter-die contact uses a sphere proxy; tray contact uses the rounded box support.
    for (int i = 0; i < count; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            auto& a = bodies[i];
            auto& b = bodies[j];
            if ((a.sleeping || a.settling) && (b.sleeping || b.settling))
            {
                continue;
            }
            Vec3 delta = b.die.position - a.die.position;
            double d = delta.length();
            if (d >= 1.56 || d < 1e-8)
            {
                continue;
            }
            Vec3 n = delta * (1 / d);
            double overlap = (1.56 - d) * 0.5;
            a.die.position = a.die.position - n * overlap;
            b.die.position = b.die.position + n * overlap;
            double closing = (b.velocity - a.velocity).dot(n);
            if (closing < 0)
            {
                Vec3 impulse = n * (-0.72 * closing);
                a.velocity = a.velocity - impulse;
                b.velocity = b.velocity + impulse;
                if (-closing > 0.55)
                {
                    ++pairHits;
                    pairImpact = std::max(pairImpact, -closing);
                    a.sleeping = b.sleeping = a.settling = b.settling = false;
                    a.quiet = b.quiet = 0;
                    Vec3 slip = (b.velocity - a.velocity) - n * (b.velocity - a.velocity).dot(n);
                    Vec3 spin = n.cross(slip) * 0.2;
                    a.angular = a.angular + spin;
                    b.angular = b.angular + spin;
                }
            }
            constrain(a);
            constrain(b);
        }
    }
}

bool ThrowSimulation::step(double dt, bool single)
{
    if (!rolling || (paused && !single) || dt <= 0 || !std::isfinite(dt))
    {
        return false;
    }
    int n = std::max(1, static_cast<int>(std::ceil(std::min(dt, 0.25) * 240)));
    double h = std::min(dt, 0.25) / n;
    for (int s = 0; s < n; s++)
    {
        integrate(h);
    }
    bool sleeping = true;
    for (int i = 0; i < count; i++)
    {
        if (!bodies[i].sleeping)
        {
            sleeping = false;
        }
    }
    if (!sleeping)
    {
        return false;
    }
    rolling = false;
    ThrowRecord record;
    record.id = history.empty() ? 1 : history.back().id + 1;
    record.count = count;
    record.power = lastPower;
    record.duration = elapsed;
    record.tableHits = tableHits;
    record.wallHits = wallHits;
    for (int i = 0; i < count; i++)
    {
        bodies[i].die.value = topFace(bodies[i].die.rotation);
        record.values[i] = bodies[i].die.value;
    }
    history.push_back(record);
    return true;
}

long long ThrowSimulation::cumulative() const
{
    long long total = 0;
    for (auto r : history)
    {
        total += r.total();
    }
    return total;
}

bool ThrowSimulation::save(const std::filesystem::path& file) const
{
    std::error_code ec;
    std::filesystem::create_directories(file.parent_path(), ec);
    if (ec)
    {
        return false;
    }
    auto temp = file;
    temp += ".tmp";
    std::ofstream out(temp);
    if (!out)
    {
        return false;
    }
    out << "id,count,power,d1,d2,d3,total,duration,table_hits,wall_hits\n" << std::setprecision(8);
    for (auto r : history)
    {
        out << r.id << ',' << r.count << ',' << r.power << ',' << r.values[0] << ',' << r.values[1]
            << ',' << r.values[2] << ',' << r.total() << ',' << r.duration << ',' << r.tableHits
            << ',' << r.wallHits << '\n';
    }
    out.close();
    if (!out)
    {
        return false;
    }
    auto backup = file;
    backup += ".bak";
    bool exists = std::filesystem::exists(file, ec);
    if (ec)
    {
        return false;
    }
    if (exists)
    {
        std::filesystem::remove(backup, ec);
        if (ec)
        {
            return false;
        }
        std::filesystem::rename(file, backup, ec);
        if (ec)
        {
            return false;
        }
    }
    std::filesystem::rename(temp, file, ec);
    if (ec)
    {
        if (exists)
        {
            std::error_code restore;
            std::filesystem::rename(backup, file, restore);
        }
        return false;
    }
    if (exists)
    {
        std::filesystem::remove(backup, ec);
    }
    return true;
}

bool ThrowSimulation::load(const std::filesystem::path& file)
{
    if (rolling)
    {
        return false;
    }
    std::ifstream in(file);
    std::string line;
    if (!std::getline(in, line))
    {
        return false;
    }
    if (!line.empty() && line.back() == '\r')
    {
        line.pop_back();
    }
    if (line != "id,count,power,d1,d2,d3,total,duration,table_hits,wall_hits")
    {
        return false;
    }
    std::vector<ThrowRecord> records;
    unsigned previous = 0;
    while (std::getline(in, line))
    {
        if (line.empty() || line == "\r")
        {
            continue;
        }
        std::replace(line.begin(), line.end(), ',', ' ');
        std::istringstream row(line);
        ThrowRecord r;
        int sum = 0;
        std::string extra;
        if (!(row >> r.id >> r.count >> r.power >> r.values[0] >> r.values[1] >> r.values[2] >>
              sum >> r.duration >> r.tableHits >> r.wallHits) ||
            (row >> extra))
        {
            return false;
        }
        if (r.id <= previous || r.id > 100000 || r.count < 1 || r.count > 3 ||
            !std::isfinite(r.power) || r.power < 0 || r.power > 1 || !std::isfinite(r.duration) ||
            r.duration <= 0 || r.duration > 10 || r.tableHits < 0 || r.wallHits < 0)
        {
            return false;
        }
        for (int i = 0; i < 3; i++)
        {
            if (i < r.count ? (r.values[i] < 1 || r.values[i] > 6) : r.values[i] != 0)
            {
                return false;
            }
        }
        if (r.total() != sum)
        {
            return false;
        }
        previous = r.id;
        records.push_back(r);
        if (records.size() > 100000)
        {
            return false;
        }
    }
    if (!in.eof())
    {
        return false;
    }
    history = std::move(records);
    if (!history.empty())
    {
        const auto& last = history.back();
        setCount(last.count);
        for (int i = 0; i < count; i++)
        {
            bodies[i].die.rotation = faceUp(last.values[i], 0.18);
            bodies[i].die.value = last.values[i];
        }
        lastPower = last.power;
    }
    return true;
}
} // namespace dice
