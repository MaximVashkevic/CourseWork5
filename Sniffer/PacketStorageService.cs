using System;
using System.Data.SQLite;

namespace Sniffer
{
    public sealed class PacketStorageService : IDisposable
    {
        private const string DataSourcePrefix = "Data Source=";
        private const string MemoryDataSource = DataSourcePrefix + ":memory:";

        private const string CreatePacketTableCommandText =
            "CREATE TABLE IF NOT EXISTS packets(" +
                "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                "seconds INTEGER NOT NULL," +
                "microseconds INTEGER NOT NULL," +
                "linkLayerType INTEGER NOT NULL," +
                "captureData BLOB NOT NULL" +
            ");";
        private const string InsertPacketCommandText =
            "INSERT INTO packets(seconds, microseconds, linkLayerType, captureData) VALUES(@seconds, @microseconds, @linkLayerType, @captureData);";
        private const string SelectAllPacketsCommandText = "SELECT * FROM packets;";
        private const string SecondsParameterName = "@seconds";
        private const string MicrosecondsParameterName = "@microseconds";
        private const string LinkLayerTypeParameterName = "@linkLayerType";
        private const string CaptureDataParameterName = "@captureData";
        private const int IDParameterIndex = 0;
        private const int SecondsParameterIndex = IDParameterIndex + 1;
        private const int MicrosecondsParameterIndex = SecondsParameterIndex + 1;
        private const int LinkLayerTypeParameterIndex = MicrosecondsParameterIndex + 1;
        private const int CaptureDataParameterIndex = LinkLayerTypeParameterIndex + 1;

        private SQLiteConnection _connection;
        private SQLiteCommand _insertPacketCommand;
        private SQLiteCommand _selectAllPacketsCommand;
        private readonly object _lock;

        public delegate void GetPacketsCallback(Packet packet);

        public PacketStorageService()
        {
            _lock = new object();
            _connection = new SQLiteConnection(MemoryDataSource);
            _connection.Open();
            using (var createPacketTableCommand = new SQLiteCommand(CreatePacketTableCommandText, _connection))
            {
                createPacketTableCommand.ExecuteNonQuery();
            }
        }

        public PacketStorageService(string fileName)
        {
            var dataSource = DataSourcePrefix + fileName;
            _connection = new SQLiteConnection(dataSource);
            _connection.Open();
            using (var createPacketTableCommand = new SQLiteCommand(CreatePacketTableCommandText, _connection))
            {
                createPacketTableCommand.ExecuteNonQuery();
            }
        }

        public void AddPacket(Packet packet)
        {
            lock (_lock)
            {
                if (_insertPacketCommand == null)
                {
                    _insertPacketCommand = new SQLiteCommand(InsertPacketCommandText, _connection);
                }
                _insertPacketCommand.Parameters.AddWithValue(CaptureDataParameterName, packet.Data);
                _insertPacketCommand.ExecuteNonQuery();
            }
        }

        public void GetPackets(GetPacketsCallback getPacketsCallback)
        {
            if (_selectAllPacketsCommand == null)
            {
                _selectAllPacketsCommand = new SQLiteCommand(SelectAllPacketsCommandText, _connection);
            }

            lock (_lock)
            {
                using (SQLiteDataReader reader = _selectAllPacketsCommand.ExecuteReader())
                {
                    while (reader.Read())
                    {
                        var id = reader.GetInt32(IDParameterIndex);
                        var captureData = reader.GetFieldValue<byte[]>(CaptureDataParameterIndex);
                        var packet = new Packet(id, captureData);
                        getPacketsCallback(packet);
                    }
                }
            }
        }

        public void SavePackets(string path)
        {
            var saveDatabaseDataSource = DataSourcePrefix + path;
            using (var saveDatabaseConnection = new SQLiteConnection(saveDatabaseDataSource))
            {
                saveDatabaseConnection.Open();
                lock (_lock)
                {
                    _connection.BackupDatabase(saveDatabaseConnection, "main", "main", -1, null, -1);
                }
            }
        }

        public void Dispose()
        {
            DisposeManagedResources();
        }

        private void DisposeManagedResources()
        {
            _connection?.Dispose();
            _connection = null;

            _insertPacketCommand?.Dispose();
            _insertPacketCommand = null;

            _selectAllPacketsCommand?.Dispose();
            _selectAllPacketsCommand = null;
        }
    }
}
