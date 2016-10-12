<?php
// GENERATED CODE -- DO NOT EDIT!

namespace MyGame\Example\ {

  class MonsterStorageClient extends \Grpc\BaseStub {
  
    /**
     * @param string $hostname hostname
     * @param array $opts channel options
     * @param Grpc\Channel $channel (optional) re-use channel object
     */
    public function __construct($hostname, $opts, $channel = null) {
      parent::__construct($hostname, $opts, $channel);
    }
    
    /**
     * @param Monster $argument input argument
     * @param array $metadata metadata
     * @param array $options call options
     */
    public function Store(Monster $argument, $metadata = [], $options = []) {
      return $this->_simpleRequest('/MonsterStorage/Store',
      $argument,
      'data',
      'MyGame\Example\Stat::getRootAsStatFromBytes',
      $metadata, $options);
    }
    
    /**
     * @param Stat $argument input argument
     * @param array $metadata metadata
     * @param array $options call options
     */
    public function Retrieve(Stat $argument, $metadata = [], $options = []) {
      return $this->_simpleRequest('/MonsterStorage/Retrieve',
      $argument,
      'data',
      'MyGame\Example\Monster::getRootAsMonsterFromBytes',
      $metadata, $options);
    }
    
  }
  
}
