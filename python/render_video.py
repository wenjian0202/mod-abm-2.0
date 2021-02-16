import numpy as np
import matplotlib.pyplot as plt
from matplotlib import animation

from PIL import Image

import sys
import yaml


# Constants.
DPI = 200


def load_config(path_to_config_file):
    """ Load the config in yaml from config file. """
    with open(path_to_config_file) as file:
        config = yaml.load(file, Loader=yaml.FullLoader)

        assert(config["output_config"]["datalog_config"]["output_datalog"]
               and "The output_datalog flag is off! Make sure the simulation is run with output_datalog on in order to be able to render video!")
        assert(config["output_config"]["video_config"]["render_video"]
               and "The render_video flag is off! Make sure the simulation is run with render_video on in order to be able to render video!")

        return config


def convert_to_xs_and_ys(waypoint, lon_min, lon_max, lat_min, lat_max, w, h):
    xs = []
    ys = []

    for step in waypoint:
        xs.append((step["lon"] - lon_min) / (lon_max - lon_min) * w)
        ys.append((lat_max - step["lat"]) / (lat_max - lat_min) * h)

    return (xs, ys)


def convert_to_x_and_y(pos, lon_min, lon_max, lat_min, lat_max, w, h):
    x = (pos["lon"] - lon_min) / (lon_max - lon_min) * w
    y = (lat_max - pos["lat"]) / (lat_max - lat_min) * h

    return (x, y)


def get_color(id):
    color = "0.50"
    if id == 0:
        color = "#dc241f"
    elif id == 1:
        color = "#9b0058"
    elif id == 2:
        color = "#0019a8"
    elif id == 3:
        color = "#0098d8"
    elif id == 4:
        color = "#b26300"
    return color


def main():
    # Check command line arguments.
    if (len(sys.argv) != 3):
        print("[ERROR] We need exact 2 arguments aside from the program name for correct execution! \n"
              "- Usage: python3 <prog name> <arg1> <arg2>. \n"
              "  <arg1> is the path to the platform config file. \n"
              "  <arg2> is the path to the background map image. \n"
              "- Example: python3 {} \"./config/platform.yml\" \"./media/hongkong.png\"\n".format(sys.argv[0]))

        sys.exit(1)

    print("Loading the config from {}.".format(sys.argv[1]))

    config = load_config(sys.argv[1])

    lon_min = config["area_config"]["lon_min"]
    lon_max = config["area_config"]["lon_max"]
    lat_min = config["area_config"]["lat_min"]
    lat_max = config["area_config"]["lat_max"]

    print("Loaded boundary: lon_min={}, lon_max={}, lat_min={}, lat_max={}.".format(
        lon_min, lon_max, lat_min, lat_max))

    img = Image.open(sys.argv[2])
    w, h = img.size

    print("Loaded background map image from {}: width={}, height={}".format(
        sys.argv[2], w, h))

    path_to_datalog = config["output_config"]["datalog_config"]["path_to_output_datalog"]
    with open(path_to_datalog) as file:
        datalog = yaml.load(file, Loader=yaml.FullLoader)
        num_frames = len(datalog) - 1

        assert(num_frames > 0
               and "The input datalog is empty! Make sure the simulation is run correctly when generating the datalog!")

        num_vehs = len(datalog[0]["vehicles"])

        assert(num_vehs > 0
               and "The input datalog has no vehicle data! Make sure the simulation is run correctly when generating the datalog!")

        print("Loaded the datalog from {}. It has {} vehicles running for {} frames.".format(
            path_to_datalog, num_vehs, num_frames))

        # Create the plot.
        fig = plt.figure(figsize=(w/DPI, h/DPI), dpi=DPI)
        ax = plt.axes(xlim=(0, w), ylim=(h, 0))
        ax.imshow(img)
        ax.spines['top'].set_visible(False)
        ax.spines['left'].set_visible(False)
        ax.spines['bottom'].set_visible(False)
        ax.spines['right'].set_visible(False)
        fig.subplots_adjust(left=0.00, bottom=0.00, right=1.00, top=1.00)

        # Handles for vehicles, the immediate next waypoint, the following waypoint, and further waypoints.
        vehs = []
        wp0 = []
        wp1 = []
        wp2 = []

        # Also plot the dispatched trips and the walked away trips.
        frame_interval_s = config["simulation_config"]["cycle_s"] / \
            config["output_config"]["video_config"]["frames_per_cycle"]
        dispatched_trips = ax.plot([], [], 'P', color='darkgreen',
                                   marker='P', markersize=8, alpha=0.6)[0]
        walked_away_trips = ax.plot([], [], 'X', color='darkred',
                                    marker='X', markersize=8, alpha=0.6)[0]

        # A text box
        text = ax.text(0.02 * w, 0.95 * h, "text", horizontalalignment='left', verticalalignment='bottom', fontsize=12,
                       bbox=dict(facecolor='white', edgecolor='grey', boxstyle='round', alpha=0.3))

        for id, vehicle in enumerate(datalog[0]["vehicles"]):
            # Get the color of the current vehicle. We only color the first 5 vehicles for visibility.
            color = get_color(id)
            vehs.append(ax.plot([], [], color=color,
                                marker='o', markersize=8, alpha=0.6)[0])
            wp0.append(ax.plot([], [], linestyle='-', linewidth=2,
                               color=color, alpha=0.6)[0])
            wp1.append(ax.plot([], [], linestyle='--', linewidth=2,
                               color=color, alpha=0.6)[0])
            wp2.append(ax.plot([], [], linestyle=':', linewidth=2,
                               color=color, alpha=0.6)[0])

        def animate(n):
            vehicles = datalog[n]["vehicles"]
            for id, vehicle in enumerate(vehicles):
                vehs[id].set_data(convert_to_x_and_y(
                    vehicle["pos"], lon_min, lon_max, lat_min, lat_max, w, h))

                wp0[id].set_data([], [])
                wp1[id].set_data([], [])
                wp2[id].set_data([], [])

                if (vehicle["waypoints"] is None):
                    continue

                if (len(vehicle["waypoints"]) > 0):
                    xs, ys = convert_to_xs_and_ys(
                        vehicle["waypoints"][0], lon_min, lon_max, lat_min, lat_max, w, h)
                    wp0[id].set_data(xs, ys)

                if (len(vehicle["waypoints"]) > 1):
                    xs, ys = convert_to_xs_and_ys(
                        vehicle["waypoints"][1], lon_min, lon_max, lat_min, lat_max, w, h)
                    wp1[id].set_data(xs, ys)

                if (len(vehicle["waypoints"]) > 2):
                    xs, ys = convert_to_xs_and_ys(
                        vehicle["waypoints"][2], lon_min, lon_max, lat_min, lat_max, w, h)
                    wp2[id].set_data(xs, ys)

            system_time_s = datalog[n]["system_time_s"]
            trips = datalog[num_frames]["trips"]
            dispatched_trips_xs = []
            dispatched_trips_ys = []
            walked_away_trips_xs = []
            walked_away_trips_ys = []

            total_trips_count = 0
            accepted_trips_count = 0
            completed_trips_count = 0

            for trip in trips:
                if trip["request_time_s"] >= system_time_s:
                    break

                total_trips_count += 1

                if trip["status"] != "WALKAWAY":
                    accepted_trips_count += 1
                if trip["status"] == "COMPLETE":
                    completed_trips_count += 1

                if trip["status"] == "WALKAWAY":
                    if trip["request_time_s"] >= system_time_s - frame_interval_s:
                        x, y = convert_to_x_and_y(
                            trip["origin"], lon_min, lon_max, lat_min, lat_max, w, h)
                        walked_away_trips_xs.append(x)
                        walked_away_trips_ys.append(y)
                elif trip["status"] != "COMPLETE" or system_time_s <= trip["pickup_time_s"]:
                    x, y = convert_to_x_and_y(
                        trip["origin"], lon_min, lon_max, lat_min, lat_max, w, h)
                    dispatched_trips_xs.append(x)
                    dispatched_trips_ys.append(y)

            dispatched_trips.set_data(dispatched_trips_xs, dispatched_trips_ys)
            walked_away_trips.set_data(
                walked_away_trips_xs, walked_away_trips_ys)

            text_str = "T = {}s\n{} requested trips ({} accepted, {} completed)".format(
                system_time_s,
                total_trips_count,
                accepted_trips_count,
                completed_trips_count)
            text.set_text(text_str)

            return vehs, wp0, wp1, wp2, dispatched_trips, walked_away_trips, text

        anime = animation.FuncAnimation(fig, animate, frames=num_frames)

        # Set up formatting for the movie file and write.
        fps = 1 / frame_interval_s * \
            config["output_config"]["video_config"]["replay_speed"]
        path_to_output_video = config["output_config"]["video_config"]["path_to_output_video"]
        Writer = animation.writers['ffmpeg']
        writer = Writer(fps=fps, metadata=dict(
            artist='mod-abm-2.0'), bitrate=1800)
        anime.save(path_to_output_video, writer=writer)

        print("Video saved at {}. FPS={}, DPI={}.".format(
            path_to_output_video, fps, DPI))


if __name__ == "__main__":
    main()
