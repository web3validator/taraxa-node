#include "config.hpp"

#include <json/json.h>
#include <libdevcore/LevelDB.h>

#include <fstream>

namespace taraxa {

std::string getConfigDataAsString(Json::Value root, std::vector<string> path) {
  try {
    for (auto i = 0; i < path.size(); i++) {
      root = root[path[i]];
    }
    return root.asString();
  } catch (...) {
    std::cerr << "Error in processing configuration file on param: ";
    for (auto i = 0; i < path.size(); i++) std::cerr << path[i] << ".";
    std::cerr << std::endl;
    throw;
  }
}

uint32_t getConfigDataAsUInt(Json::Value root, std::vector<string> path,
                             bool optional = false) {
  try {
    for (auto i = 0; i < path.size(); i++) {
      if (optional && root.isNull()) return 0;
      root = root[path[i]];
    }
    return root.asUInt();
  } catch (...) {
    std::cerr << "Error in processing configuration file on param: ";
    for (auto i = 0; i < path.size(); i++) std::cerr << path[i] << ".";
    std::cerr << std::endl;
    throw;
  }
}

uint64_t getConfigDataAsUInt64(Json::Value root, std::vector<string> path) {
  try {
    for (auto i = 0; i < path.size(); i++) {
      root = root[path[i]];
    }
    return root.asUInt();
  } catch (...) {
    std::cerr << "Error in processing configuration file on param: ";
    for (auto i = 0; i < path.size(); i++) std::cerr << path[i] << ".";
    std::cerr << std::endl;
    throw;
  }
}

FullNodeConfig::FullNodeConfig(std::string const &json_file)
    : json_file_name(json_file) {
  try {
    Json::Value root;
    std::ifstream config_doc(json_file, std::ifstream::binary);
    config_doc >> root;

    node_secret = getConfigDataAsString(root, {"node_secret"});
    vrf_secret = getConfigDataAsString(root, {"vrf_secret"});
    db_path = getConfigDataAsString(root, {"db_path"});
    dag_processing_threads =
        getConfigDataAsUInt(root, {"dag_processing_threads"});

    network.network_address = getConfigDataAsString(root, {"network_address"});
    network.network_id = getConfigDataAsString(root, {"network_id"});
    network.network_listen_port =
        getConfigDataAsUInt(root, {"network_listen_port"});
    network.network_simulated_delay =
        getConfigDataAsUInt(root, {"network_simulated_delay"});
    network.network_transaction_interval =
        getConfigDataAsUInt(root, {"network_transaction_interval"});
    network.network_bandwidth =
        getConfigDataAsUInt(root, {"network_bandwidth"});
    network.network_ideal_peer_count =
        getConfigDataAsUInt(root, {"network_ideal_peer_count"});
    network.network_max_peer_count =
        getConfigDataAsUInt(root, {"network_max_peer_count"});
    network.network_sync_level_size =
        getConfigDataAsUInt(root, {"network_sync_level_size"});
    network.network_encrypted =
        getConfigDataAsUInt(root, {"network_encrypted"}) != 0;
    network.network_performance_log =
        getConfigDataAsUInt(root, {"network_performance_log"}) & 1;
    if (getConfigDataAsUInt(root, {"network_performance_log"}) & 2)
      dev::db::LevelDB::setPerf(true);
    for (auto &item : root["network_boot_nodes"]) {
      NodeConfig node;
      node.id = getConfigDataAsString(item, {"id"});
      node.ip = getConfigDataAsString(item, {"ip"});
      node.port = getConfigDataAsUInt(item, {"port"});
      network.network_boot_nodes.push_back(node);
    }
    rpc.address =
        boost::asio::ip::address::from_string(network.network_address);
    rpc.port = getConfigDataAsUInt(root, {"rpc_port"});
    rpc.ws_port = getConfigDataAsUInt(root, {"ws_port"});
    {  // for test experiments
      test_params.max_transaction_queue_warn = getConfigDataAsUInt(
          root, {"test_params", "max_transaction_queue_warn"}, true);
      test_params.max_transaction_queue_drop = getConfigDataAsUInt(
          root, {"test_params", "max_transaction_queue_drop"}, true);

      test_params.max_block_queue_warn = getConfigDataAsUInt(
          root, {"test_params", "max_block_queue_warn"}, true);

      test_params.block_proposer.mode = getConfigDataAsString(
          root, {"test_params", "block_proposer", "mode"});
      test_params.block_proposer.shard =
          getConfigDataAsUInt(root, {"test_params", "block_proposer", "shard"});
      test_params.block_proposer.transaction_limit = getConfigDataAsUInt(
          root, {"test_params", "block_proposer", "transaction_limit"});
      if (test_params.block_proposer.mode == "random") {
        test_params.block_proposer.min_freq = getConfigDataAsUInt(
            root,
            {"test_params", "block_proposer", "random_params", "min_freq"});
        test_params.block_proposer.max_freq = getConfigDataAsUInt(
            root,
            {"test_params", "block_proposer", "random_params", "max_freq"});
      } else if (test_params.block_proposer.mode == "sortition") {
        test_params.block_proposer.difficulty_bound =
            getConfigDataAsUInt(root, {"test_params", "block_proposer",
                                       "sortition_params", "difficulty_bound"});
        test_params.block_proposer.lambda_bits =
            getConfigDataAsUInt(root, {"test_params", "block_proposer",
                                       "sortition_params", "lambda_bits"});
      } else {
        std::cerr << "Unknown propose mode: "
                  << test_params.block_proposer.mode;
        assert(false);
      }
      test_params.pbft.lambda_ms_min =
          getConfigDataAsUInt(root, {"test_params", "pbft", "lambda_ms_min"});
      test_params.pbft.committee_size =
          getConfigDataAsUInt(root, {"test_params", "pbft", "committee_size"});
      test_params.pbft.valid_sortition_coins = getConfigDataAsUInt64(
          root, {"test_params", "pbft", "valid_sortition_coins"});
      test_params.pbft.dag_blocks_size =
          getConfigDataAsUInt(root, {"test_params", "pbft", "dag_blocks_size"});
      test_params.pbft.ghost_path_move_back = getConfigDataAsUInt(
          root, {"test_params", "pbft", "ghost_path_move_back"});
      test_params.pbft.skip_periods =
          getConfigDataAsUInt64(root, {"test_params", "pbft", "skip_periods"});
      test_params.pbft.run_count_votes =
          getConfigDataAsUInt(root, {"test_params", "pbft", "run_count_votes"});
    }
    // TODO parse from json:
    // Either a string name of a predefined config,
    // or the full json of a custom config
    chain = decltype(chain)::DEFAULT();
    configured = true;
  } catch (std::exception &e) {
    std::cerr << "Error in processing configuration file: " << std::endl
              << json_file << " Exception:" << e.what() << std::endl;
  }
}

RpcConfig::RpcConfig(std::string const &json_file) : json_file_name(json_file) {
  try {
    Json::Value root;
    std::ifstream config_doc(json_file, std::ifstream::binary);
    config_doc >> root;
    port = root["port"].asUInt();
    ws_port = root["ws_port"].asUInt();
    address = boost::asio::ip::address::from_string(root["address"].asString());
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
}

std::ostream &operator<<(std::ostream &strm, NodeConfig const &conf) {
  strm << "  [Node Config] " << std::endl;
  strm << "    node_id: " << conf.id << std::endl;
  strm << "    node_ip: " << conf.ip << std::endl;
  strm << "    node_port: " << conf.port << std::endl;
  return strm;
}

std::ostream &operator<<(std::ostream &strm, NetworkConfig const &conf) {
  strm << "[Network Config] " << std::endl;
  strm << "  json_file_name: " << conf.json_file_name << std::endl;
  strm << "  network_address: " << conf.network_address << std::endl;
  strm << "  network_listen_port: " << conf.network_listen_port << std::endl;
  strm << "  network_simulated_delay: " << conf.network_simulated_delay
       << std::endl;
  strm << "  network_transaction_interval: "
       << conf.network_transaction_interval << std::endl;
  strm << "  network_bandwidth: " << conf.network_bandwidth << std::endl;
  strm << "  network_ideal_peer_count: " << conf.network_ideal_peer_count
       << std::endl;
  strm << "  network_max_peer_count: " << conf.network_max_peer_count
       << std::endl;
  strm << "  network_sync_level_size: " << conf.network_sync_level_size
       << std::endl;
  strm << "  network_id: " << conf.network_id << std::endl;

  strm << "  --> boot nodes  ... " << std::endl;
  for (auto const &c : conf.network_boot_nodes) {
    strm << c << std::endl;
  }
  return strm;
}

std::ostream &operator<<(std::ostream &strm, FullNodeConfig const &conf) {
  strm << std::ifstream(conf.json_file_name).rdbuf() << std::endl;
  return strm;
}
}  // namespace taraxa