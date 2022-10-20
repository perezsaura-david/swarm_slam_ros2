#include "cslam/back_end/utils/simulated_rendezvous.h"

using namespace cslam;

SimulatedRendezVous::SimulatedRendezVous(std::shared_ptr<rclcpp::Node> &node, 
                                        const std::string& schedule_file, 
                                        const unsigned int &robot_id): node_(node), robot_id_(robot_id), enabled_(true)
{
    try{
        int64_t initial_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        std::ifstream schedule(schedule_file);
        if (schedule.is_open())
        {
            for (std::string line; std::getline(schedule, line); )
            {
                auto delim0 = line.find(",");
                RCLCPP_ERROR(node_->get_logger(), line);
                if (robot_id_ == std::stoul(line.substr(0, delim0)))
                {
                    while (delim0 != std::string::npos){
                        auto delim1 = line.find(",", delim0 + 1);

                        auto start = std::stoull(line.substr(delim0 + 1, delim1)) + initial_time + 1800;

                        delim0 = delim1;
                        delim1 = line.find(",", delim0);

                        auto end = std::stoull(line.substr(delim0 + 1, delim1)) + initial_time + 1800;

                        rendezvous_ranges_.push_back(std::make_pair(start, end));// TODO: consider the bag rate?
                        RCLCPP_ERROR(node_->get_logger(), "rendezvous_ranges_: %d %d", start, end);

                        delim0 = delim1;
                    }
                }
            }
            RCLCPP_ERROR(node_->get_logger(), "rendezvous_ranges_.size: %d", rendezvous_ranges_.size());
            schedule.close();
        }
    }
    catch (const std::exception &e)
    {
        RCLCPP_ERROR(node_->get_logger(), "Reading simulated rendezvous schedule failed: %s", e.what());
        enabled_ = false;
    }
}

bool SimulatedRendezVous::is_alive()
{
    if (enabled_) {
        bool is_alive = false;
        uint64_t current_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        for (const auto& range: rendezvous_ranges_)
        {
            if (current_time >= range.first && current_time <= range.second)
            {
                is_alive = true;
            }
        }
        return is_alive;
    }
    return true;
}