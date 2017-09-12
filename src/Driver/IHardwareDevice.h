#pragma once

#include <memory>
#include "DriverConfigParser.h"
#include <vector>

#include "protobuff_defs/HighLevelEvent.pb.h"
#include "pevent.h"

#include "PluginApis.h"
#include <boost/variant.hpp>
#include <boost/signals2.hpp>
#include <boost/mpl/map.hpp>
#include <boost/mpl/pair.hpp>
#include "BodyGraph.h"
#include "SimulatedHapticNode.h"
#include "NodeView.h"
class PluginApis;
class PluginEventSource;
class HardwareCoordinator;

struct DeviceDescriptor;

struct NodeDescriptor {
	nsvr_node_type type;
	std::string displayName;
	nsvr_node_id id;
};
class Node {
public:
	Node(const NodeDescriptor&);
	Node();
	nsvr_node_id id() const;
	std::string name() const;
	nsvr_node_type type() const;
	//is physical or virtual 
	//type of haptic


private:
	std::string m_name;
	nsvr_node_id m_id;
	nsvr_node_type m_type;


};

//raii style?
class TrackingStream {
public:
	TrackingStream(nsvr_node_id id, tracking_api* api);
	~TrackingStream();
	boost::signals2::signal<void(nsvr_node_id, nsvr_quaternion*)> Signal;
	void deliver(nsvr_quaternion* q);
private:
	nsvr_node_id m_id;
	tracking_api* m_api;
};

class HumanBodyNodes;

//I want device to be agnostic to what type of nodes it holds..
class Device {
public:
	using RequestId = uint64_t;

	Device(const DeviceDescriptor& desc, PluginApis& api, PluginEventSource& ev, Parsing::BodyGraphDescriptor bodyGraph);
	Device& operator=(const Device&) = delete;
	Device(const Device&) = delete;

	std::string name() const;
	nsvr_device_id id() const;

	void simulate(double dt);

	void deliverRequest(const NullSpaceIPC::HighLevelEvent& event);
	
	void registerTrackedObjects(const boost::signals2::signal<void(nsvr_node_id, nsvr_quaternion*)>::slot_type&);

	std::vector<NodeView> renderDevices() const;

	
private:
	void setupDynamicBodyRepresentation();
	nsvr_device_concept m_concept;
	std::string m_name;

	std::unordered_map<nsvr_node_id, SimulatedHapticNode> m_simulatedNodes;
	std::unordered_map<nsvr_node_id, std::unique_ptr<TrackingStream>> m_trackedObjects;
	std::vector<std::unique_ptr<Node>> m_nodes;


	PluginApis* m_apis;
	
	
	void dynamicallyFetchNodes();
	void parseNodes(const std::vector<NodeDescriptor>& descriptor);
	void createNewNode(const NodeDescriptor& descriptor);
	void fetchNodeInfo(nsvr_node_id node_id);


	void handleSimpleHaptic(RequestId id, const ::NullSpaceIPC::SimpleHaptic& simple);
	void handlePlaybackEvent(RequestId id, const ::NullSpaceIPC::PlaybackEvent& event);
	
	const Node* findNode(nsvr_node_id id) const;
	Node* findNode(nsvr_node_id id);

	BodyGraph m_graph;

	nsvr_device_id m_deviceId;
	std::atomic<bool> m_isBodyGraphSetup;

	 


	void setupInitialBodyRepresentation(const Parsing::BodyGraphDescriptor& bodyGraph);
	void handleRealtimeEvent(uint64_t request_id, const NullSpaceIPC::RealtimeHaptic& realtime);
	void handleCurveHaptic(uint64_t request_id, const NullSpaceIPC::CurveHaptic& curve);
};



