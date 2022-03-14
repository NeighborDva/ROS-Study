#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Int16.h"
#include "sensor_msgs/Imu.h"
#include "sensor_msgs/NavSatFix.h"
#include "geometry_msgs/Vector3.h" 
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include "marvelmind_nav/hedge_pos_a.h"
#include <math.h>

//distance
double distance;
double cal_seq1;
double cal_seq2;
double  r=6371;
//initial_pos
geometry_msgs::Vector3 initial_pos;
geometry_msgs::Vector3 initial_pos_utm;

int ip=0;


//utm 변환을 위한 변수선언

int mag_north=105;
double roll,pitch,yaw,yaw_d,ag,ag_d;
double yaw_tmp;
geometry_msgs::Vector3 outdoor_gps_currentPos;
geometry_msgs::Vector3 outdoor_gps_utm_currentPos;
geometry_msgs::Vector3 indoor_gps;

//utm 변환을 위한 상수 변수선언
//토픽을 받아와서 초기화 해주는 것을 이용하는 것은 해당 콜백메소드에서 초기화 할것, 안그러면 엉뚱한 값나옴
//데이터 자료
double lat_rad;
double equatorial_radius=6378137; //적도반경,엑셀 기준 a
double polar_radius = 6356752.3142;//극반지름,엑셀기준 b
int utm_zone = 52;
double delta_long; //경도_델타, 엑셀기준 p(rad)
//delta_long= 경도 - (52*6-183)// utm zone이 52이기 때문에 이런식으로 계산
double scale_factor = 0.9996; //엑셀기준 k0
double ecc_n = (equatorial_radius-polar_radius)/(equatorial_radius+polar_radius); //eccentricity
double ecc_e = sqrt(1-pow((polar_radius/equatorial_radius),2.0));
double ecc_e2 =ecc_e*ecc_e/(1-ecc_e*ecc_e);
double r_curv_2; //엑셀에서 nu

//수직호길이계산, Meridional Arc
double MA_A =  equatorial_radius*(1-ecc_n+(5*ecc_n*ecc_n/4.0)*(1-ecc_n)+(81*pow(ecc_n,4.0)/64.0)*(1-ecc_n));
double MA_B = (3*equatorial_radius*ecc_n/2.0)*(1-ecc_n-(7*ecc_n*ecc_n/8.0)*(1-ecc_n)+55*pow(ecc_n,4.0)/64.0);
double MA_C = (15*equatorial_radius*ecc_n*ecc_n/16.0)*(1-ecc_n+(3*ecc_n*ecc_n/4.0)*(1-ecc_n));
double MA_D = (35*equatorial_radius*pow(ecc_n,3.0)/48.0)*(1-ecc_n+11*ecc_n*ecc_n/16.0);
double MA_E = (315*equatorial_radius*pow(ecc_n,4.0)/51.0)*(1-ecc_n);
double S ; 

//utm좌표변환을 위한 계수
double Ki;
double Kii;
double Kiii;
double Kiv;
double Kv;

void poseCallback(const sensor_msgs::NavSatFix::ConstPtr& msg)
{
	outdoor_gps_currentPos.x = (double)msg->latitude;
	outdoor_gps_currentPos.y = (double)msg->longitude;
    outdoor_gps_currentPos.z =(double)msg->altitude;
      
     //utm
    lat_rad=outdoor_gps_currentPos.x*M_PI/180;
    delta_long=(outdoor_gps_currentPos.y-(utm_zone*6-183))*M_PI/180;
    r_curv_2 = equatorial_radius/pow((1-pow(ecc_e*sin(lat_rad),2.0)),0.5);
    S= MA_A*lat_rad-MA_B*sin(2*lat_rad)+MA_C*sin(4*lat_rad)-MA_D*sin(6*lat_rad)+MA_E*sin(8*lat_rad);
    
    Ki=S*scale_factor;
    Kii=r_curv_2*sin(lat_rad)*cos(lat_rad)*scale_factor/2;
    Kiii=((r_curv_2*sin(lat_rad)*pow(cos(lat_rad),3.0)/24)*(5-pow(tan(lat_rad),2.0)+9*ecc_e2*pow(cos(lat_rad),2.0)+4*pow(ecc_e2,2.0)*pow(cos(lat_rad),4.0)))*scale_factor;
    Kiv=r_curv_2*cos(lat_rad)*scale_factor;
    Kv=pow(cos(lat_rad),3.0)*(r_curv_2/6)*(1-pow(tan(lat_rad),2.0)+ecc_e2*pow(cos(lat_rad),2.0))*scale_factor;
    //이상없음
   //ROS_INFO("lat_rad: %f",lat_rad);
   //ROS_INFO("Delta_long: %f",delta_long);
    //ROS_INFO("ecc_e: %f ecc_e2: %f ecc_n: %f r_curv_2: %f",ecc_e, ecc_e2, ecc_n, r_curv_2);
		//ROS_INFO("MA_A: %f MA_B: %f MA_C: %f MA_D: %f MA_E: %f S: %f",MA_A, MA_B ,MA_C ,MA_D ,MA_E ,S);
    //ROS_INFO("Ki: %f Kii:%f Kiii:%f Kiv:%f Kv:%f",Ki,Kii,Kiii,Kiv,Kv);
     
    //double utm_x=Ki+Kii*delta_long*delta_long+Kiii*pow(delta_long,4.0);
    //ROS_INFO("utm_x:%f ",utm_x);
    
    
    outdoor_gps_utm_currentPos.x= 500000+(Kiv*delta_long+Kv*pow(delta_long,3.0));//utm Easting
    outdoor_gps_utm_currentPos.y=(Ki+Kii*delta_long*delta_long+Kiii*pow(delta_long,4.0));//utm northing
     if(ip==0)
    {
		initial_pos.x=outdoor_gps_currentPos.x;
		initial_pos.y=outdoor_gps_currentPos.y;
		
		initial_pos_utm.x = outdoor_gps_utm_currentPos.x;
		initial_pos_utm.y = outdoor_gps_utm_currentPos.y;
		ip=1;
	}
	//거리계산
	//cal_seq1=sin(fabs(outdoor_gps.x-initial_pos.x)*M_PI/180/2)*sin(fabs(outdoor_gps.x-initial_pos.x)*M_PI/180/2)+cos(initial_pos.x*M_PI/180)*cos(outdoor_gps.x*M_PI/180)*pow(sin(fabs(outdoor_gps.y-initial_pos.y)*M_PI/180/2),2.0);
    //ROS_INFO("cal_seq1 %.20lf", cal_seq1);
    //cal_seq2=2*atan2(sqrt(1-cal_seq1),sqrt(cal_seq1));
    //ROS_INFO("cal_seq2 %.20lf", cal_seq2);
    //distance=r*cal_seq;
    //distance = acos(cos(initial_pos.x*M_PI/180)*cos(outdoor_gps.x*M_PI/180))+sin(initial_pos.x*M_PI/180)*sin(outdoor_gps.x*M_PI/180)*cos((outdoor_gps.y-initial_pos.y)*M_PI/180)*r;
}

void poseCallback2(const marvelmind_nav::hedge_pos_a& hedge_pos_msg)
{ 
				indoor_gps.x=(double)hedge_pos_msg.x_m;   
				indoor_gps.y=(double)hedge_pos_msg.y_m;
				indoor_gps.z=(double)hedge_pos_msg.z_m;
}


void imuCallback(const sensor_msgs::Imu::ConstPtr& msg) {
	tf2::Quaternion q(
        msg->orientation.x,
        msg->orientation.y,
        msg->orientation.z,
        msg->orientation.w);
      tf2::Matrix3x3 m(q);
 
      m.getRPY(roll, pitch, yaw);
      
      yaw_d=yaw*180/3.14159;//+mag_north;
      /*ag=msg->angular_velocity.z;
      ag_d= (double)ag*180/3.14159;*/
      
  ROS_INFO( "Yaw_Q : %6.3lf Yaw_E : %6.3lf Yaw_d : %6.3lf ag: %6.3lf ag_d: %6.3lf",msg->orientation.z,yaw,yaw_d);
}



int main(int argc, char **argv)
{
  char buf[2];
  ros::init(argc, argv, "imu_gps");

  ros::NodeHandle n;
 
  
  roll = pitch = yaw = 0.0;  
  ros::Subscriber subOGps = n.subscribe("/gps/fix",10, &poseCallback);
  ros::Subscriber subIGps = n.subscribe("/hedge_pos_a",10, &poseCallback2);
   ros::Subscriber subImu = n.subscribe("/handsfree/imu", 10, &imuCallback);
  
 
  ros::Rate loop_rate(10);  // 10
  
 
  while (ros::ok())
  {	
	 if(outdoor_gps.x!=NULL){
	//ROS_INFO("Outdoor GPS lat : %6.3lf   long : %6.3lf  alt:%6.3lf  Yaw_d: %6.3lf %dN Utm_x:%6.3lf Utm_y:%6.3lf", outdoor_gps.x, outdoor_gps.y, outdoor_gps.z, yaw_d, utm_zone,outdoor_gps_utm.x,outdoor_gps_utm.y); 
  ROS_INFO("Initial Pos lat : %.7lf   long : %.7lf UTM %d N %.7lf %.7lf",initial_pos.x,initial_pos.y,utm_zone,initial_pos_utm.x,initial_pos_utm.y);
   ROS_INFO("Outdoor GPS lat : %.7lf   long : %.7lf UTM %d N %.7lf %.7lf", outdoor_gps.x, outdoor_gps.y,utm_zone,outdoor_gps_utm.x,outdoor_gps_utm.y); 
  ROS_INFO("Distance : %.8lf",distance);
   } 
	 
	 if(indoor_gps.x!=NULL)
	ROS_INFO("Indoor GPS x : %6.3lf   y : %6.3lf  z:%6.3lf  Yaw_d: %6.3lf ", indoor_gps.x, indoor_gps.y, indoor_gps.z, yaw_d); 
	
	loop_rate.sleep();
    ros::spinOnce();
   
  }
  return 0;
}
