#!/usr/bin/env ruby

require 'open3'
require_relative './bitshares_api.rb'

class BitSharesNode

  attr_reader :rpc_instance, :command

  class Error < RuntimeError; end

  def initialize(client_binary, options)
    @client_binary, @options = client_binary, options
    @handler = nil

    @command = @client_binary.clone
    @command << " --data-dir #{@options[:data_dir]}"
    @command << " --genesis-config #{@options[:genesis]}"
    @command << " --min-delegate-connection-count=0"
    @command << " --server"
    @command << " --rpcuser=user"
    @command << " --rpcpassword=pass"
    @command << " --httpport=#{@options[:http_port]}"
    @command << " --upnp=false"
    if @options[:delegate]
      @command << " --p2p-port=10000"
    else
      @command << " --connect-to=127.0.0.1:10000"
    end
    @command << " --disable-default-peers"
  end
  
  def start
    puts "starting client: #{@command}"

    stdin, out, wait_thr = Open3.popen2e(@command)
    @handler = {stdin: stdin, out: out, wait_thr: wait_thr}
    while true
      if select([out], nil, nil, 0.2) and out.eof?
        puts "process exited and doesn't have output queued."
        break
      else
        line = out.gets
        puts line
        break if line.include? "Starting HTTP JSON RPC server on port #{@options[:http_port]}"
      end
    end

    sleep 1.0
    
    @rpc_instance = BitShares::API::Rpc.new(@options[:http_port], 'user', 'pass', ignore_errors: false)

    return

  end

  def exec(method, *params)
    raise Error, "rpc instance is not defined, make sure the node is started" unless @rpc_instance
    @rpc_instance.request(method, params)
  end

  def wait_new_block
    inititial_block_num = new_block_num = exec('info')['blockchain_head_block_num']
    while inititial_block_num == new_block_num
      sleep 1.0
      new_block_num = exec('info')['blockchain_head_block_num']
    end
  end

end

if $0 == __FILE__
  client_binary = '/Users/vz/bitshares/osx_build/programs/client/bitshares_client'
  client_node = BitSharesNode.new client_binary, data_dir: "tmp/client_a", genesis: "test_genesis.json", http_port: 5680, delegate: false
  client_node.start
  client_node.exec 'create', 'default', '123456789'
  client_node.exec 'unlock', '9999999', '123456789'
  puts "client node is up and running on port 5680"
  STDIN.getc
end