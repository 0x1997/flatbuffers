import grpc
from grpc.beta import implementations as beta_implementations
from grpc.beta import interfaces as beta_interfaces
from grpc.framework.common import cardinality
from grpc.framework.interfaces.face import utilities as face_utilities


class MonsterStorageStub(object):
  
  def __init__(self, channel):
    """Constructor.
    
    Args:
      channel: A grpc.Channel.
    """
    self.Store = channel.unary_unary(
        '/MyGame.Example..MonsterStorage/Store',
        request_serializer=Monster.SerializeToString,
        response_deserializer=Stat.FromString,
        )
    self.Retrieve = channel.unary_unary(
        '/MyGame.Example..MonsterStorage/Retrieve',
        request_serializer=Stat.SerializeToString,
        response_deserializer=Monster.FromString,
        )


class MonsterStorageServicer(object):
  
  def Store(self, request, context):
    context.set_code(grpc.StatusCode.UNIMPLEMENTED)
    context.set_details('Method not implemented!')
    raise NotImplementedError('Method not implemented!')
  
  def Retrieve(self, request, context):
    context.set_code(grpc.StatusCode.UNIMPLEMENTED)
    context.set_details('Method not implemented!')
    raise NotImplementedError('Method not implemented!')


def add_MonsterStorageServicer_to_server(servicer, server):
  rpc_method_handlers = {
      'Store': grpc.unary_unary_rpc_method_handler(
          servicer.Store,
          request_deserializer=Monster.FromString,
          response_serializer=Stat.SerializeToString,
      ),
      'Retrieve': grpc.unary_unary_rpc_method_handler(
          servicer.Retrieve,
          request_deserializer=Stat.FromString,
          response_serializer=Monster.SerializeToString,
      ),
  }
  generic_handler = grpc.method_handlers_generic_handler(
      'MyGame.Example..MonsterStorage', rpc_method_handlers)
  server.add_generic_rpc_handlers((generic_handler,))


class BetaMonsterStorageServicer(object):
  def Store(self, request, context):
    context.code(beta_interfaces.StatusCode.UNIMPLEMENTED)
  def Retrieve(self, request, context):
    context.code(beta_interfaces.StatusCode.UNIMPLEMENTED)


class BetaMonsterStorageStub(object):
  def Store(self, request, timeout, metadata=None, with_call=False, protocol_options=None):
    raise NotImplementedError()
  Store.future = None
  def Retrieve(self, request, timeout, metadata=None, with_call=False, protocol_options=None):
    raise NotImplementedError()
  Retrieve.future = None


def beta_create_MonsterStorage_server(servicer, pool=None, pool_size=None, default_timeout=None, maximum_timeout=None):
  request_deserializers = {
    ('MyGame.Example..MonsterStorage', 'Retrieve'): Stat.FromString,
    ('MyGame.Example..MonsterStorage', 'Store'): Monster.FromString,
  }
  response_serializers = {
    ('MyGame.Example..MonsterStorage', 'Retrieve'): Monster.SerializeToString,
    ('MyGame.Example..MonsterStorage', 'Store'): Stat.SerializeToString,
  }
  method_implementations = {
    ('MyGame.Example..MonsterStorage', 'Retrieve'): face_utilities.unary_unary_inline(servicer.Retrieve),
    ('MyGame.Example..MonsterStorage', 'Store'): face_utilities.unary_unary_inline(servicer.Store),
  }
  server_options = beta_implementations.server_options(request_deserializers=request_deserializers, response_serializers=response_serializers, thread_pool=pool, thread_pool_size=pool_size, default_timeout=default_timeout, maximum_timeout=maximum_timeout)
  return beta_implementations.server(method_implementations, options=server_options)


def beta_create_MonsterStorage_stub(channel, host=None, metadata_transformer=None, pool=None, pool_size=None):
  request_serializers = {
    ('MyGame.Example..MonsterStorage', 'Retrieve'): Stat.SerializeToString,
    ('MyGame.Example..MonsterStorage', 'Store'): Monster.SerializeToString,
  }
  response_deserializers = {
    ('MyGame.Example..MonsterStorage', 'Retrieve'): Monster.FromString,
    ('MyGame.Example..MonsterStorage', 'Store'): Stat.FromString,
  }
  cardinalities = {
    'Retrieve': cardinality.Cardinality.UNARY_UNARY,
    'Store': cardinality.Cardinality.UNARY_UNARY,
  }
  stub_options = beta_implementations.stub_options(host=host, metadata_transformer=metadata_transformer, request_serializers=request_serializers, response_deserializers=response_deserializers, thread_pool=pool, thread_pool_size=pool_size)
  return beta_implementations.dynamic_stub(channel, 'MyGame.Example..MonsterStorage', cardinalities, options=stub_options)
