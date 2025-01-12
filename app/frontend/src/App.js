import React, { useState, useEffect } from 'react';
import { Line } from 'react-chartjs-2';
import { Chart as ChartJS, CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend } from 'chart.js';
import './App.css';

ChartJS.register(CategoryScale, LinearScale, PointElement, LineElement, Title, Tooltip, Legend);

const BASE_URL = 'http://localhost:5000';

const App = () => {
  const [boards, setBoards] = useState([]);
  const [selectedBoard, setSelectedBoard] = useState(null);
  const [timeRange, setTimeRange] = useState('last1h');
  const [sensorReadings, setSensorReadings] = useState([]);
  const [viewMode, setViewMode] = useState('graph'); // 'graph' or 'table'
  const [boardStatus, setBoardStatus] = useState({}); // Tracks if a board is enabled or disabled

  const fetchBoards = () => {
    fetch(`${BASE_URL}/api/boards`)
      .then((res) => res.json())
      .then((data) => {
        setBoards(data);
        const initialStatus = {};
        data.forEach((board) => {
          initialStatus[board.boardId] = true;
        });
        setBoardStatus(initialStatus);
      })
      .catch((err) => console.error('Error fetching boards:', err));
  };

  const fetchReadings = () => {
    if (selectedBoard) {
      fetch(`${BASE_URL}/api/boards/${selectedBoard}/data?range=${timeRange}`)
        .then((res) => res.json())
        .then((data) => setSensorReadings(data))
        .catch((err) => console.error('Error fetching sensor readings:', err));
    }
  };

  useEffect(() => {
    fetchBoards();
  }, []);

  useEffect(() => {
    fetchReadings();
    const interval = setInterval(fetchReadings, 30000);
    return () => clearInterval(interval);
  }, [selectedBoard, timeRange]);

  const toggleBoardStatus = (boardId) => {
    const isCurrentlyEnabled = boardStatus[boardId];
    const newStatus = !isCurrentlyEnabled;
    const topic = newStatus ? 'enable' : 'disable';

    fetch(`${BASE_URL}/api/boards/${boardId}/${topic}`, {
      method: 'POST',
    })
      .then((res) => {
        if (res.ok) {
          setBoardStatus((prevStatus) => ({
            ...prevStatus,
            [boardId]: newStatus,
          }));
        } else {
          alert(`Failed to ${topic} board ${boardId}.`);
        }
      })
      .catch((err) => console.error(`Error toggling board status: ${err}`));
  };

  const toggleAlarm = async () => {
    try {
      const response = await fetch(`${BASE_URL}/api/boards/${selectedBoard}/alarm`, {
        method: 'GET',
      });
      await response.json()
    } catch (error) {
      console.error('Error toggling alarm:', error);
    }
  };

  const createChartData = (label, color, key) => ({
    labels: sensorReadings.map((reading) => new Date(reading.timestamp).toLocaleTimeString()),
    datasets: [
      {
        label,
        data: sensorReadings.map((reading) => reading[key]),
        borderColor: color,
        backgroundColor: `${color}33`,
        fill: true,
      },
    ],
  });

  const chartOptions = (title) => ({
    responsive: true,
    plugins: {
      legend: {
        position: 'top',
      },
      title: {
        display: true,
        text: title,
      },
      tooltip: {
        mode: 'index',
        intersect: false,
      },
    },
    interaction: {
      mode: 'nearest',
      axis: 'x',
      intersect: false,
    },
  });

  return (
    <div className="app-container">
      <div className="sidebar">
        <h2>Boards</h2>
        <ul className="board-list">
          {boards.map((board) => (
            <li key={board.boardId} className={`board-item ${selectedBoard === board.boardId ? 'active' : ''}`}>
              <div onClick={() => setSelectedBoard(board.boardId)}>{board.boardId}</div>
            </li>
          ))}
        </ul>
        {selectedBoard && (
          <button
            className={`toggle-status-button ${boardStatus[selectedBoard] ? 'disable' : 'enable'}`}
            onClick={() => toggleBoardStatus(selectedBoard)}
          >
            {boardStatus[selectedBoard] ? 'Disable' : 'Enable'} Board
          </button>
        )}
        {selectedBoard && (
          <button
          className={`alarm-button active }`}
          onClick={toggleAlarm}
          >
            {'Activate Alarm'}
          </button>
        )}
        <div className="time-range">
          <h3>Time Range</h3>
          <select value={timeRange} onChange={(e) => setTimeRange(e.target.value)}>
            <option value="last5m">Last 5 Minutes</option>
            <option value="last30m">Last 30 Minutes</option>
            <option value="last1h">Last 1 Hour</option>
            <option value="last6h">Last 6 Hours</option>
            <option value="last12h">Last 12 Hours</option>
            <option value="last24h">Last 24 Hours</option>
            <option value="last7d">Last 7 Days</option>
          </select>
        </div>
        <div className="view-mode">
          <h3>View Mode</h3>
          <button className={`view-button ${viewMode === 'graph' ? 'active' : ''}`} onClick={() => setViewMode('graph')}>
            Graph
          </button>
          <button className={`view-button ${viewMode === 'table' ? 'active' : ''}`} onClick={() => setViewMode('table')}>
            Table
          </button>
        </div>
        <button className="refresh-button" onClick={fetchReadings}>
          Refresh Data
        </button>
      </div>

      <div className="main-content">
        {selectedBoard ? (
          <>
            <h1>Board: {selectedBoard}</h1>
            {viewMode === 'graph' ? (
              <div className="graphs">
                <div className="graph">
                  <Line data={createChartData('Temperature', 'rgba(255, 99, 132)', 'temperature')} options={chartOptions('Temperature Over Time')} />
                </div>
                <div className="graph">
                  <Line data={createChartData('Pressure', 'rgba(54, 162, 235)', 'pressure')} options={chartOptions('Pressure Over Time')} />
                </div>
                <div className="graph">
                  <Line data={createChartData('Air Quality', 'rgba(75, 192, 192)', 'airQuality')} options={chartOptions('Air Quality Over Time')} />
                </div>
              </div>
            ) : (
              <div className="table-container">
                <table>
                  <thead>
                    <tr>
                      <th>Timestamp</th>
                      <th>Temperature (°C)</th>
                      <th>Pressure (hPa)</th>
                      <th>Air Quality (ppm)</th>
                    </tr>
                  </thead>
                  <tbody>
                    {sensorReadings.map((reading) => (
                      <tr key={reading.timestamp}>
                        <td>{new Date(reading.timestamp).toLocaleString()}</td>
                        <td>{reading.temperature}</td>
                        <td>{reading.pressure}</td>
                        <td>{reading.airQuality}</td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            )}
          </>
        ) : (
          <p>Select a board to view sensor data.</p>
        )}
      </div>
    </div>
  );
};

export default App;
