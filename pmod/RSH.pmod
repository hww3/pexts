
constant cvs_version = "$Id $";

object bind;
string host;
string remote_user;
string local_user;
array cmd;
int port;
function read_callback;
function close_callback;
int local_port;
object data;
object srv;

void connect( string _host, string _local_user, string _remote_user, array _cmd, function _read_callback, function _close_callback, void|int _port ) {
  host = _host;
  cmd = _cmd;
  port = (_port?_port:514);
  read_callback = _read_callback;
  close_callback = _close_callback;
  local_user = _local_user;
  remote_user = _remote_user;
  bind = Stdio.Port();
  srv = Stdio.File();
  srv->connect( host, port );
  bind->bind( 1022, accept );
  local_port = (bind->query_address() / " ")[ 1 ];
  srv->write( (string)local_port + "\0" );
}

void accept( mixed id ) {
  data = bind->accept( id );
  data->close();
  srv->write( local_user + "\0" + remote_user + "\0" + ( cmd * " " ) + "\0" );
  srv->read(1);
  srv->set_id( srv->query_address() );
  srv->set_nonblocking( _read, 0, _close );
  bind = 0;
}

void _close( mixed id ) {
  srv->close();
  close_callback();
}

void _read( mixed id, string data ) {
  read_callback( data );
}
