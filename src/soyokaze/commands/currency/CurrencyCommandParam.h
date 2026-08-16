#pragma once

class Settings;

namespace launcherapp { namespace commands { namespace currency {

class CommandParam
{
public:
    bool Save(Settings& settings) const;
    bool Load(Settings& settings);

    bool mIsEnable{false};
};

}}}
