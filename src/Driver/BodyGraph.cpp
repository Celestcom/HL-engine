//#include "stdafx.h"
#include "BodyGraph.h"
#include <boost/graph/iteration_macros.hpp>
#include <boost/log/trivial.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#undef _USE_MATH_DEFINES

#include "nsvr_core_errors.h"


const segment_range segment_range::full = { 0, 1 };
const segment_range segment_range::lower_half = { 0, 0.5 };
const segment_range segment_range::upper_half = { 0.5, 1.0 };

const angle_range angle_range::full = { 0, 360 };
const angle_range angle_range::left_half = { 180, 360 };
const angle_range angle_range::right_half = { 0, 180 };
const angle_range angle_range::front_half = { 270, 90 };
const angle_range angle_range::back_half = { 90, 270 };

BodyGraph::BodyGraph()
{
	using region = subregion::shared_region;

	auto entire_torso = std::shared_ptr<subregion>(new 
		subregion(region::identifier_torso, segment_range::full, angle_range::full, {
			subregion(region::identifier_torso_front, segment_range::full, angle_range::front_half,{
				subregion(region::identifier_chest_left,  segment_range{ 0.8, 1.0 }, angle_range{ 340, 360 }),
				subregion(region::identifier_middle_sternum, segment_range{0.45, 0.55}, angle_range{355, 5}),
				subregion(region::identifier_upper_ab_left,  segment_range{ 0.6, 0.8 }, angle_range{ 340, 360 }),
				subregion(region::identifier_middle_ab_left, segment_range{ 0.4, 0.6 }, angle_range{ 340, 360 }),
				subregion(region::identifier_lower_ab_left,  segment_range{ 0.0, 0.4 }, angle_range{ 340, 360 }),
				subregion(region::identifier_chest_right, segment_range{ 0.8, 1.0 }, angle_range{ 0, 20 }),
				subregion(region::identifier_upper_ab_right, segment_range{ 0.6, 0.8 }, angle_range{ 0, 20 }),
				subregion(region::identifier_middle_ab_right, segment_range{ 0.4, 0.6 }, angle_range{ 0, 20 }),
				subregion(region::identifier_lower_ab_right, segment_range{ 0.0, 0.4 }, angle_range{ 0, 20 })
			}),
			subregion(region::identifier_torso_back, segment_range::full, angle_range::back_half,{
				subregion(region::identifier_upper_back_left,  segment_range{ 0.5, 1.0 }, angle_range{ 180, 270 }),
				subregion(region::identifier_upper_back_right,  segment_range{ 0.5, 1.0 }, angle_range{ 90, 180 })
			}),
			subregion(region::identifier_torso_left, segment_range::full, angle_range::left_half),
			subregion(region::identifier_torso_right, segment_range::full, angle_range::right_half)
	}));


	auto upper_arm_left = std::shared_ptr<subregion>(new
		subregion(region::identifier_upper_arm_left, segment_range::full, angle_range::full, {
			subregion(region::identifier_shoulder_left, segment_range{ 0.85, 1.0 }, angle_range::full)
		}));


	auto upper_arm_right = std::shared_ptr<subregion>(new
		subregion(region::identifier_upper_arm_right, segment_range::full, angle_range::full, {
			subregion(region::identifier_shoulder_right, segment_range{ 0.85, 1.0 }, angle_range::full)
		}));



	auto lower_arm_left = std::shared_ptr<subregion>(new
		subregion(region::identifier_lower_arm_left, segment_range::full, angle_range::full));


	auto lower_arm_right = std::shared_ptr<subregion>(new
		subregion(region::identifier_lower_arm_right, segment_range::full, angle_range::full));
	

	auto upper_leg_left = std::shared_ptr<subregion>(new
		subregion(region::identifier_upper_leg_left, segment_range::full, angle_range::full));

	auto lower_leg_left = std::shared_ptr<subregion>(new
		subregion(region::identifier_lower_leg_left, segment_range::full, angle_range::full));

	auto upper_leg_right = std::shared_ptr<subregion>(new
		subregion(region::identifier_upper_leg_right, segment_range::full, angle_range::full));

	auto lower_leg_right = std::shared_ptr<subregion>(new
		subregion(region::identifier_lower_leg_right, segment_range::full, angle_range::full));

	auto head = std::shared_ptr<subregion>(new
		subregion(region::identifier_head, segment_range::full, angle_range::full));
	
	auto left_palm = std::shared_ptr<subregion>(new
		subregion(region::identifier_palm_left, segment_range::full, angle_range::full));

auto right_palm = std::shared_ptr<subregion>(new
	subregion(region::identifier_palm_right, segment_range::full, angle_range::full));

m_bodyparts.emplace(nsvr_bodypart_head, Bodypart(nsvr_bodypart_head, 19.03, head));

m_bodyparts.emplace(nsvr_bodypart_torso, Bodypart(nsvr_bodypart_torso, 51.9, entire_torso));

m_bodyparts.emplace(nsvr_bodypart_upperarm_left, Bodypart(nsvr_bodypart_upperarm_left, 29.41, upper_arm_left));

m_bodyparts.emplace(nsvr_bodypart_lowerarm_left, Bodypart(nsvr_bodypart_lowerarm_left, 27.68, lower_arm_left));

m_bodyparts.emplace(nsvr_bodypart_upperarm_right, Bodypart(nsvr_bodypart_upperarm_right, 29.41, upper_arm_right));

m_bodyparts.emplace(nsvr_bodypart_lowerarm_right, Bodypart(nsvr_bodypart_lowerarm_right, 27.68, lower_arm_right));

m_bodyparts.emplace(nsvr_bodypart_lowerarm_right, Bodypart(nsvr_bodypart_lowerarm_right, 27.68, lower_arm_right));

m_bodyparts.emplace(nsvr_bodypart_lowerleg_left, Bodypart(nsvr_bodypart_lowerleg_left, 43.25, lower_leg_left));

m_bodyparts.emplace(nsvr_bodypart_upperleg_left, Bodypart(nsvr_bodypart_upperleg_left, 43.25, upper_leg_left));

m_bodyparts.emplace(nsvr_bodypart_lowerleg_right, Bodypart(nsvr_bodypart_lowerleg_right, 43.25, lower_leg_right));

m_bodyparts.emplace(nsvr_bodypart_upperleg_right, Bodypart(nsvr_bodypart_upperleg_right, 43.25, upper_leg_right));


m_bodyparts.emplace(nsvr_bodypart_palm_left, Bodypart(nsvr_bodypart_palm_left, 10.0, left_palm));
m_bodyparts.emplace(nsvr_bodypart_palm_right, Bodypart(nsvr_bodypart_palm_right, 10.0, right_palm));

for (auto& bp : m_bodyparts) {
	bp.second.region->init_backlinks();
}

}

subregion::subregion()
	: region(shared_region::identifier_unknown)
	, seg{ 0, 0 }
	, ang{ 0,0 }
	, coords{ 0, 0, 0 }
	, children()
	, hardware_defined_regions()
	, parent(nullptr) {}

subregion::subregion(shared_region region, segment_range segment_offset, angle_range angle_range)
	: region(region)
	, seg(segment_offset)
	, ang(angle_range)
	, children()
	, hardware_defined_regions()
	, parent(nullptr) {
	calculateCoordinates();

}

subregion::subregion(shared_region region, segment_range segment_offset, angle_range angle_range, std::vector<subregion> child_regions)
	: region(region)
	, seg(segment_offset)
	, ang(angle_range)
	, children(std::move(child_regions))
	, hardware_defined_regions()
	, parent(nullptr) {

	calculateCoordinates();


}
int BodyGraph::CreateNode(const char * name, nsvr_bodygraph_region * pose)
{
	//To create a new node:
		//First, add a vertex into the graph with the given name.
	boost::add_vertex(name, m_nodes);

	//Then, grab the bodypart corresponding with the given arguments.
	auto& bodypart = m_bodyparts[pose->bodypart];

	//Then ask that bodypart to find the nearest match to the given coordinates.
	subregion::shared_region region = bodypart.region->find_best_match(pose->segment_ratio, pose->rotation).second;

	//Update the vertex with the information that was found
	m_nodes[name] = NodeData(name, *pose, region);

	//Update the bodypart's subregion that corresponded with the match to have a reference to this graph node.
	bodypart.region->find(region)->hardware_defined_regions.push_back(name);



	BOOST_LOG_TRIVIAL(info) << "[BodyGraph] Node [" << name << "] registered on region " << region._to_string();
	return 0;
}


int BodyGraph::ConnectNodes(const char* a, const char* b)
{
	if (m_nodes.vertex(a) != LabeledGraph::null_vertex() && m_nodes.vertex(b) != LabeledGraph::null_vertex()) {
		boost::add_edge_by_label(a, b, m_nodes);
		return nsvr_success;
	}
	else {
		return nsvr_error_nosuchnode;
	}
}

int BodyGraph::Associate(const char * node, nsvr_node_id node_id)
{
	if (m_nodes.vertex(node) != LabeledGraph::null_vertex()) {
		m_nodes[node].addNode(node_id);
		return nsvr_success;
	}
	else {
		return nsvr_error_nosuchnode;
	}

}
	

int BodyGraph::Unassociate(const char * node, nsvr_node_id node_id)
{
	if (m_nodes.vertex(node) != LabeledGraph::null_vertex()) {
		m_nodes[node].removeNode(node_id);
		return nsvr_success;
	}
	else {
		return nsvr_error_nosuchnode;
	}
}



void BodyGraph::ClearAssociations(nsvr_node_id node_id)
{
	BGL_FORALL_VERTICES_T(v, m_nodes, LabeledGraph) {
		m_nodes.graph()[v].removeNode(node_id);
	}
}


std::vector<nsvr_node_id> BodyGraph::getNodesForNamedRegion(subregion::shared_region region) const
{
	std::vector<nsvr_node_id> nodes;

	//Special case for whole body - must refactor, and test these methods better
	if (region._value == subregion::shared_region::identifier_body) {
		const auto& all = getAllNodes();
		for (const auto& kvp : all) {
			nodes.insert(nodes.end(), kvp.second.begin(), kvp.second.end());
		}
		std::sort(nodes.begin(), nodes.end());
		nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());
		return nodes;

	}


	for (auto& bp : m_bodyparts) {
		subregion* ptr = bp.second.region->find(region);
		if (ptr != nullptr) {
			
			//If this region doesn't have any devices associated with it,
			//Then we need to traverse upwards until we find one that does!
			while (ptr->hardware_defined_regions.empty()) {
				if (ptr->parent != nullptr) {
					ptr = ptr->parent;
				}
				else {
					break;
				}
			}
			
			//Okay, we found some references - for each one, grab the associated nodes out of the graph
			//and insert into our list.
			for (const std::string& name : ptr->hardware_defined_regions) {
				auto& found = m_nodes[name].nodes;
				nodes.insert(nodes.end(), found.begin(), found.end());
			}
		}
	}

	std::sort(nodes.begin(), nodes.end());
	nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());
	return nodes;
	
}


std::vector<subregion::shared_region::_enumerated> BodyGraph::getRegionsForNode(nsvr_node_id node) const
{
	std::vector<subregion::shared_region::_enumerated> regions;
	BGL_FORALL_VERTICES_T(v, m_nodes, LabeledGraph) {
		subregion::shared_region region = m_nodes.graph()[v].computed_region;
		auto& nodeList = m_nodes.graph()[v].nodes;
		if (std::find(nodeList.begin(), nodeList.end(), node) != nodeList.end()) {
			regions.push_back(region);
		}

	}
	return regions;
}


std::unordered_map<subregion::shared_region::_enumerated, std::vector<nsvr_node_id>> BodyGraph::getAllNodes() const
{

	std::unordered_map<subregion::shared_region::_enumerated, std::vector<nsvr_node_id>> nodes;

	//Walk through the graph, and add any nodes that were found to the hashtable
	BGL_FORALL_VERTICES_T(v, m_nodes, LabeledGraph) {
		subregion::shared_region region = m_nodes.graph()[v].computed_region;
		auto& nodeList = m_nodes.graph()[v].nodes;
		auto& currentDevices = nodes[region];
		currentDevices.insert(currentDevices.end(), nodeList.begin(), nodeList.end());
	}

	//make sure each list has no duplicates
	for (auto& kvp : nodes) {
		auto& list = kvp.second;
		std::sort(list.begin(), list.end());
		list.erase(std::unique(list.begin(), list.end()), list.end());
	}

	return nodes;
}



void BodyGraph::NodeData::addNode(nsvr_node_id id)
{
	auto it = std::find(nodes.begin(), nodes.end(), id);
	if (it == nodes.end()) {
		nodes.push_back(id);
	}
}

void BodyGraph::NodeData::removeNode(nsvr_node_id id)
{
	auto it = std::remove(nodes.begin(), nodes.end(), id);
	nodes.erase(it, nodes.end());
}

