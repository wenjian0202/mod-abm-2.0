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
    if id == 0:
        return 'crimson'
    if id == 1:
        return 'darkorange'
    if id == 2:
        return 'gold'
    if id == 1:
        return 'darkgreen'
    if id == 1:
        return 'royalblue'
    return 'grey'


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
        num_frames = len(datalog)

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

        for id, vehicle in enumerate(datalog[0]["vehicles"]):
            # Get the color of the current vehicle. We only color the first 5 vehicles for visibility.
            color = get_color(id)
            vehs.append(ax.plot([], [], color=color,
                                marker='o', markersize=5, alpha=0.7)[0])
            wp0.append(ax.plot([], [], linestyle='-',
                               color=color, alpha=0.7)[0])
            wp1.append(ax.plot([], [], linestyle='--',
                               color=color, alpha=0.7)[0])
            wp2.append(ax.plot([], [], linestyle=':',
                               color=color, alpha=0.4)[0])

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

            return vehs, wp0, wp1, wp2

        anime = animation.FuncAnimation(fig, animate, frames=num_frames)

        # Set up formatting for the movie file and write.
        frame_interval_s = config["simulation_config"]["cycle_s"] / config["output_config"]["video_config"]["frames_per_cycle"]
        fps = 1 / frame_interval_s * config["output_config"]["video_config"]["replay_speed"]
        path_to_output_video = config["output_config"]["video_config"]["path_to_output_video"]
        Writer = animation.writers['ffmpeg']
        writer = Writer(fps=fps, metadata=dict(
            artist='mod-abm-2.0'), bitrate=1800)
        anime.save(path_to_output_video, writer=writer)

        print("Video saved at {} using fps={}.".format(path_to_output_video, fps))


if __name__ == "__main__":
    main()
