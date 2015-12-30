#include "csoko_thinker/csoko_thinker.h"

#include <cstdlib>
#include <ctime>




//	<include file="$(find stdr_robot)/launch/robot_manager.launch" />

//<node type="stdr_server_node" pkg="stdr_server" name="stdr_server" output="screen" args="$(find csoko_resources)/map/sokobanMap1.yaml"/>
//<node pkg="tf" type="static_transform_publisher" name="world2map" args="0 0 0 0 0 0  world map 100" />
//<include file="$(find stdr_gui)/launch/stdr_gui.launch"/>

using namespace csoko_thinker;

int main(int argc,char **argv)
{
	ros::init(argc, argv, "csoko_thinker_node", ros::init_options::AnonymousName);
	string res_path(argv[1]);
	CSokoFrame::setPathToResources(res_path);
	CSoko_Thinker obj(argc, argv);
	ros::spin();

	while(true)
	{}	
	return 0;
}

/**
 * Load map, create frame, start timer
 */
namespace csoko_thinker{

CSoko_Thinker::CSoko_Thinker(int argc,char **argv)
{
	odom_state = 0;
	string res_path(argv[1]);
	string name = res_path + argv[2];

	CSokoFrame::setPathToResources(res_path);

	ROS_ERROR("MAP NAME: %s\n", name.c_str());
	loadMap(name);

	CSokoFrame::setPathToResources(res_path);

//	ros::Timer timer = nh.createTimer(ros::Duration(0.1), onUpdate);
	ros::Timer timer = nh.createTimer(ros::Duration(0.1), &CSoko_Thinker::timerCallback,this);
	///TODO START LOGIC
	
	/*
    if(argc != 3)
    {
      ROS_ERROR(
        "Usage : csoko_thinker csoko_thinker <robot_frame_id> <laser_frame_id>");
      exit(0);
    }

    std::string robot_topic = std::string("/") + std::string(argv[1]);

    laser_topic = robot_topic + std::string("/") + std::string(argv[2]);
    speeds_topic = robot_topic + std::string("/cmd_vel");
    odom_topic = robot_topic +  std::string("/odom");

    odom_state = 0;

    laser_sub = n_.subscribe(laser_topic.c_str(), 1, &CSoko_Thinker::callback, this);
    odom_sub =   n_.subscribe(odom_topic.c_str(), 1, &CSoko_Thinker::odometryCallback, this);

    cmd_vel_pub = n_.advertise<geometry_msgs::Twist>(speeds_topic.c_str(), 1);
	 */

	occ_grid_topic = "map";
	occ_grid_sub = nh.subscribe(occ_grid_topic.c_str(), 1, &CSoko_Thinker::mapCallback, this);

	frame.signalUpdate(grid,objects);
}


CSoko_Thinker::~CSoko_Thinker(void)
{

}

void CSoko_Thinker::onUpdate(const ros::TimerEvent&)
{
	ros::spinOnce();
	updateMap();
	update();
	if (!ros::ok())
	{void onUpdate();
	}
}

void CSoko_Thinker::timerCallback(const ros::TimerEvent& e)
{
	ROS_ERROR("CALLBACK");
	ROS_INFO("Timer callback!");
}

void CSoko_Thinker::updateMap() {
	//TODO NAVIGATION LOGIC
}

void CSoko_Thinker::update()
{
//	map.drawAll(frame);
}

void CSoko_Thinker::mapCallback(const nav_msgs::OccupancyGrid& msg){
	this->occ_grid_msg = msg;
	ROS_INFO("received map!");

	this->map_height = msg.info.height;
	this->map_width = msg.info.width;
	ROS_INFO("map_height: %i\nmap_width: %i", this->map_height, this->map_width);



	this->my_map = new int8_t*[this->map_height];
	for(int i = 0; i < this->map_height; i++){
		my_map[i] = new int8_t[this->map_width];

		for(int j = 0; j < this->map_width; j++){
			my_map[i][j] = msg.data.at(i*j);
		}
	}

	if(CSOKO_THINKER_DEBUG){
		ROS_INFO("CSOKO_THINKER_DEBUG");
		try{

			std::ofstream outfile ("map.csv");
			for(int i = 0; i < this->map_height; i++){
				for(int j = 0; j < this->map_width; j++){
					outfile << (int) my_map[i][j] << ",";
				}
				outfile << std::endl;
			}
			outfile.close();
		} catch(...){
			ROS_INFO("Exception!");
		}

	}
}


void CSoko_Thinker::callback(const sensor_msgs::LaserScan& msg)
{
	laser_scan_msg = msg;
	float linear = 0, rotational = 0;
	for(unsigned int i = 0 ; i < laser_scan_msg.ranges.size() ; i++)
	{
		float real_dist = laser_scan_msg.ranges[i];
		linear -= cos(laser_scan_msg.angle_min + i * laser_scan_msg.angle_increment)
        								/ (1.0 + real_dist * real_dist);
		rotational -= sin(laser_scan_msg.angle_min + i * laser_scan_msg.angle_increment)
        								/ (1.0 + real_dist * real_dist);
	}
	geometry_msgs::Twist cmd;

	linear /= laser_scan_msg.ranges.size();
	rotational /= laser_scan_msg.ranges.size();

	//~ ROS_ERROR("%f %f",linear,rotational);

	if(linear > 0.3)
	{
		linear = 0.3;
	}
	else if(linear < -0.3)
	{
		linear = -0.3;
	}

	cmd.linear.x = (odom_state < 3) ? 0.3 + linear : 0;
	cmd.angular.z = (odom_state > 1) ? 0.0174532925 * 10 : rotational;

	cmd_vel_pub.publish(cmd);
}


void CSoko_Thinker::odometryCallback(const nav_msgs::Odometry msg){
	if(odom_state == 0){
		odom_msg = msg;
		odom_state = 1;
		return;
	}

	float currentXPos = msg.pose.pose.position.x;
	float currentYPos = msg.pose.pose.position.y;
	float oldXPos = odom_msg.pose.pose.position.x;
	float oldYPos = odom_msg.pose.pose.position.y;
	float delta = 0.0000001;

	bool stuckX = ((currentXPos - oldXPos) == 0) ? true : false;
	bool stuckY = ((currentYPos - oldYPos) == 0) ? true: false;

	float dx = std::abs(currentXPos - oldXPos),
			dy = std::abs(currentYPos - oldYPos);

	if(stuckX && stuckY){

		if(odom_state == 1) odom_state = 2;
		else if(odom_state == 2)  odom_state = 3;
		else if(odom_state == 3) odom_state = 2;

		ROS_INFO("dx = %f ; dy =  %f ; odomState= %i", dx, dy, odom_state);
	}
	else{
		odom_state = 1;
		ROS_INFO("dx = %f ; dy =  %f ; odomState= %i", dx, dy, odom_state);
	}
	//odom_msg.pose.pose.position.z = 0.0;

	//save it
	odom_msg = msg;
}

void CSoko_Thinker::loadMap(string mapName)
{
	string line;
	int row = 0;
	string mapFilePath = mapName;
	frame.loadMap(mapName);

	ifstream mapFile(mapFilePath.c_str());
	if(mapFile.is_open())
	{
		while(getline(mapFile,line))
		{
			std::vector<CSokoTile> mapRow;
			for(int i=0;i<line.length(); i++)
			{
				if(line[i] == '-')
				{
					mapRow.push_back(CSokoTile(i,row,false, false));
				}
				else if(line[i] == 'P')
				{
					mapRow.push_back(CSokoTile(i,row,false, true));
				}
				else if(line[i] == 'R')
				{
					CSokoTile tile = CSokoTile(i,row,false, false);
					CSokoObject r(i,row,false);
					objects.push_back(r);
					tile.setObject();
					mapRow.push_back(tile);
				}
				else if(line[i] == 'C')
				{
					CSokoTile tile = CSokoTile(i,row,false, false);
					CSokoObject b(i,row,true);
					objects.push_back(b);
					tile.setObject();
					mapRow.push_back(tile);
				}
			}
			grid.push_back(mapRow);
			row++;
		}
		mapFile.close();
	}
	else {
		cout << "Unable to find map file." << endl;
	}
}

void CSoko_Thinker::drawAll()
{
	
}


}
