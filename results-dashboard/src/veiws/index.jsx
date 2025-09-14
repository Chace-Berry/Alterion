import React, { useEffect, useState } from "react";

const POLL_INTERVAL = 3000;
const JSON_PATH = "/results/lexer-results.json";

function fetchResults(setResults, setError) {
  fetch(JSON_PATH)
    .then((res) => {
      if (!res.ok) {
        throw new Error(`HTTP ${res.status}: ${res.statusText}`);
      }
      return res.json();
    })
    .then((data) => {
      setResults(Array.isArray(data) ? data : []);
      setError(null);
    })
    .catch((error) => {
      console.error('Failed to fetch results:', error);
      setError(error.message);
      setResults([]);
    });
}

export default function ResultsDashboard() {
  const [results, setResults] = useState([]);
  const [showDiffsOnly, setShowDiffsOnly] = useState(false);
  const [error, setError] = useState(null);
  const [lastUpdated, setLastUpdated] = useState(null);
  const [activeTab, setActiveTab] = useState('tokens');
  const [ast, setAst] = useState(null);
  const [astError, setAstError] = useState(null);

  useEffect(() => {
    if (activeTab === 'ast') {
      fetch('/results/ast-results.json')
        .then(res => {
          if (!res.ok) throw new Error(`HTTP ${res.status}: ${res.statusText}`);
          return res.json();
        })
        .then(data => {
          setAst(data);
          setAstError(null);
        })
        .catch(err => {
          setAstError(err.message);
          setAst(null);
        });
    }
  }, [activeTab, lastUpdated]);
  useEffect(() => {
    fetchResults(setResults, setError);
    const interval = setInterval(() => {
      fetchResults(setResults, setError);
      setLastUpdated(new Date());
    }, POLL_INTERVAL);
    return () => clearInterval(interval);
  }, []);

  const filtered = showDiffsOnly
    ? results.filter((r) => r.status === "DIFF")
    : results;

  const diffCount = results.filter((r) => r.status === "DIFF").length;
  const passCount = results.filter((r) => r.status === "OK").length;

  return (
    <div style={{
      minHeight: "100vh",
      width: "100vw",
      background: "linear-gradient(135deg, #e0eafc 0%, #cfdef3 100%)",
      display: "flex",
      flexDirection: "column",
      alignItems: "center",
      justifyContent: "flex-start",
      padding: "0",
      margin: "0",
      position: "absolute",
      top: "0",
      left: "0",
      right: "0",
      bottom: "0",
      boxSizing: "border-box",
      fontFamily: "-apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif"
    }}>
      <header style={{ 
        width: "100%", 
        background: "#343a40", 
        color: "#fff", 
        padding: "32px 0 16px 0", 
        textAlign: "center", 
        boxShadow: "0 2px 8px rgba(0,0,0,0.2)" 
      }}>
        <h1 style={{ margin: 0, fontSize: "2.5rem", letterSpacing: "2px" }}>
          Alterion Test Results Dashboard
        </h1>
        <p style={{ margin: "8px 0 0 0", fontSize: "1.2rem", color: "#b0c4de" }}>
          Live parser test results
        </p>
        {lastUpdated && (
          <p style={{ margin: "4px 0 0 0", fontSize: "0.9rem", color: "#adb5bd" }}>
            Last updated: {lastUpdated.toLocaleTimeString()}
          </p>
        )}
      </header>

      <main style={{ 
        width: "100%", 
        maxWidth: "1200px", 
        margin: "32px auto", 
        flex: 1, 
        display: "flex", 
        flexDirection: "column", 
        alignItems: "center",
        justifyContent: "flex-start",
        padding: "0 16px",
        boxSizing: "border-box"
      }}>
        {/* Tab Controls */}
        <div style={{ display: "flex", gap: "16px", marginBottom: "24px" }}>
          <button onClick={() => setActiveTab('tokens')} style={{
            padding: "10px 24px", fontWeight: "bold", fontSize: "1.1rem", borderRadius: "8px", border: "none",
            background: activeTab === 'tokens' ? "#38b2ac" : "#e0eafc", color: activeTab === 'tokens' ? "#fff" : "#343a40",
            boxShadow: "0 2px 8px rgba(0,0,0,0.1)", cursor: "pointer"
          }}>Tokens</button>
          <button onClick={() => setActiveTab('ast')} style={{
            padding: "10px 24px", fontWeight: "bold", fontSize: "1.1rem", borderRadius: "8px", border: "none",
            background: activeTab === 'ast' ? "#38b2ac" : "#e0eafc", color: activeTab === 'ast' ? "#fff" : "#343a40",
            boxShadow: "0 2px 8px rgba(0,0,0,0.1)", cursor: "pointer"
          }}>AST</button>
        </div>
        {/* Stats and Controls */}
        <div style={{ 
          display: "flex", 
          flexDirection: "row", 
          gap: "24px", 
          alignItems: "center", 
          marginBottom: "24px",
          flexWrap: "wrap",
          justifyContent: "center"
        }}>
          <div style={{ 
            background: "#fff", 
            padding: "16px 24px", 
            borderRadius: "8px", 
            boxShadow: "0 2px 8px rgba(0,0,0,0.1)",
            textAlign: "center"
          }}>
            <div style={{ fontSize: "2rem", fontWeight: "bold", color: "#2e7d32" }}>{passCount}</div>
            <div style={{ fontSize: "0.9rem", color: "#666" }}>PASS</div>
          </div>
          
          <div style={{ 
            background: "#fff", 
            padding: "16px 24px", 
            borderRadius: "8px", 
            boxShadow: "0 2px 8px rgba(0,0,0,0.1)",
            textAlign: "center"
          }}>
            <div style={{ fontSize: "2rem", fontWeight: "bold", color: "#c0392b" }}>{diffCount}</div>
            <div style={{ fontSize: "0.9rem", color: "#666" }}>DIFF</div>
          </div>

          <button
            onClick={() => setShowDiffsOnly((v) => !v)}
            style={{
              padding: "12px 28px",
              fontWeight: "bold",
              fontSize: "1.1rem",
              borderRadius: "8px",
              border: "none",
              background: showDiffsOnly ? "#ff6b6b" : "#38b2ac",
              color: "#fff",
              boxShadow: "0 2px 8px rgba(0,0,0,0.1)",
              cursor: "pointer",
              transition: "all 0.2s ease",
              transform: "translateY(0)"
            }}
            onMouseOver={(e) => e.target.style.transform = "translateY(-1px)"}
            onMouseOut={(e) => e.target.style.transform = "translateY(0)"}
          >
            {showDiffsOnly ? "Show All Results" : "Show Only DIFFs"}
          </button>
        </div>

        {/* Error Display */}
        {error && (
          <div style={{
            background: "#ffebee",
            color: "#c62828",
            padding: "16px 24px",
            borderRadius: "8px",
            marginBottom: "24px",
            width: "100%",
            maxWidth: "600px",
            textAlign: "center",
            border: "1px solid #ffcdd2"
          }}>
            <strong>Error loading results:</strong> {error}
          </div>
        )}

        {/* Results Table */}
        {activeTab === 'tokens' ? (
          filtered.length > 0 ? (
            <div style={{ 
              width: "100%", 
              maxWidth: "900px", 
              margin: "0 auto", 
              background: "#fff", 
              borderRadius: "12px", 
              boxShadow: "0 4px 16px rgba(0,0,0,0.1)",
              overflow: "hidden"
            }}>
              <table style={{ borderCollapse: "collapse", width: "100%", minWidth: "800px", color: "#111", tableLayout: "fixed" }}>
                <thead>
                  <tr style={{ background: "#f8f9fa" }}>
                    <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Index</th>
                    <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Expected Type</th>
                    <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Expected Value</th>
                    <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Returned Type</th>
                    <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Returned Value</th>
                    <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Line</th>
                    <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Column</th>
                    <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Status</th>
                  </tr>
                </thead>
              </table>
              <div style={{ maxHeight: "440px", overflowY: "auto", width: "100%" }}>
                <table style={{ borderCollapse: "collapse", width: "100%", minWidth: "800px", color: "#111", tableLayout: "fixed" }}>
                  <tbody>
                    {filtered.map((r, i) => (
                      <tr 
                        key={r.index || i} 
                        style={{ 
                          background: r.status === "DIFF" ? "#ffebee" : "#e8f5e8",
                          borderBottom: "1px solid #e9ecef",
                          color: "#111"
                        }}
                      >
                        <td style={{ padding: "12px", fontWeight: "500" }}>{r.index}</td>
                        <td style={{ padding: "12px", fontFamily: "monospace" }}>{r.expectedType}</td>
                        <td style={{ padding: "12px", fontFamily: "monospace", maxWidth: "200px", overflow: "hidden", textOverflow: "ellipsis", whiteSpace: "nowrap" }}>{r.expectedValue}</td>
                        <td style={{ padding: "12px", fontFamily: "monospace" }}>{r.returnedType}</td>
                        <td style={{ padding: "12px", fontFamily: "monospace", maxWidth: "200px", overflow: "hidden", textOverflow: "ellipsis", whiteSpace: "nowrap" }}>{r.returnedValue}</td>
                        <td style={{ padding: "12px" }}>{r.line}</td>
                        <td style={{ padding: "12px" }}>{r.column}</td>
                        <td style={{ 
                          padding: "12px", 
                          fontWeight: "bold", 
                          color: r.status === "DIFF" ? "#c0392b" : "#2e7d32"
                        }}>
                          <span style={{
                            padding: "4px 8px",
                            borderRadius: "4px",
                            fontSize: "0.875rem",
                            background: r.status === "DIFF" ? "#ffcdd2" : "#c8e6c9",
                            color: "#111"
                          }}>
                            {r.status}
                          </span>
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            </div>
          ) : (
            <div style={{ 
              marginTop: "64px", 
              color: "#888", 
              fontSize: "1.2rem", 
              textAlign: "center",
              padding: "32px",
              background: "#fff",
              borderRadius: "8px",
              boxShadow: "0 2px 8px rgba(0,0,0,0.1)"
            }}>
              {error ? "Failed to load results" : "No results found"}
            </div>
          )
        ) : activeTab === 'ast' ? (
          <div style={{ width: "100%", maxWidth: "900px", margin: "0 auto", background: "#fff", borderRadius: "12px", boxShadow: "0 4px 16px rgba(0,0,0,0.1)", overflow: "hidden", padding: "24px" }}>
            <h2 style={{ fontSize: "1.5rem", marginBottom: "16px" }}>AST (Abstract Syntax Tree)</h2>
            {astError && (
              <div style={{ background: "#ffebee", color: "#c62828", padding: "16px 24px", borderRadius: "8px", marginBottom: "24px", textAlign: "center", border: "1px solid #ffcdd2" }}>
                <strong>Error loading AST:</strong> {astError}
              </div>
            )}
            {ast && ast.children && ast.children.length > 0 ? (
              <div style={{ width: "100%", maxWidth: "900px", margin: "0 auto", background: "#fff", borderRadius: "12px", boxShadow: "0 4px 16px rgba(0,0,0,0.1)", overflow: "hidden" }}>
                <div style={{ maxHeight: "440px", overflowY: "auto", width: "100%" }}>
                  <table style={{ borderCollapse: "collapse", width: "100%", minWidth: "800px", color: "#111", tableLayout: "fixed" }}>
                    <thead>
                      <tr style={{ background: "#f8f9fa" }}>
                        <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Index</th>
                        <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Type</th>
                        <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Name</th>
                        <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Line</th>
                        <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Column</th>
                        <th style={{ padding: "16px 12px", fontSize: "1rem", fontWeight: "600", textAlign: "left", borderBottom: "2px solid #e9ecef" }}>Children</th>
                      </tr>
                    </thead>
                    <tbody>
                      {ast.children.map((node, i) => (
                        <tr key={i} style={{ background: i % 2 === 0 ? "#f8f9fa" : "#fff", borderBottom: "1px solid #e9ecef", color: "#111" }}>
                          <td style={{ padding: "12px", fontWeight: "500", textAlign: "left" }}>{i}</td>
                          <td style={{ padding: "12px", fontFamily: "monospace", textAlign: "left" }}>{node.type}</td>
                          <td style={{ padding: "12px", fontFamily: "monospace", textAlign: "left", maxWidth: "200px" }}>
                            <div style={{ maxWidth: "180px", height: "40px", overflowY: "auto", overflowX: "auto", whiteSpace: "pre", wordBreak: "break-all", borderRadius: "4px", background: "#f8f9fa", padding: "2px 6px" }}>
                              {node.name}
                            </div>
                          </td>
                          <td style={{ padding: "12px", textAlign: "left" }}>{node.line}</td>
                          <td style={{ padding: "12px", textAlign: "left" }}>{node.column}</td>
                          <td style={{ padding: "12px", textAlign: "left" }}>{node.children ? node.children.length : 0}</td>
                        </tr>
                      ))}
                    </tbody>
                  </table>
                </div>
              </div>
            ) : !astError ? (
              <div style={{ color: "#888", fontSize: "1.2rem", textAlign: "center", padding: "32px" }}>No AST found</div>
            ) : null}
          </div>
        ) : null}
      </main>

      <footer style={{ 
        width: "100%", 
        background: "#343a40", 
        color: "#fff", 
        textAlign: "center", 
        padding: "16px 0", 
        fontSize: "1rem", 
        letterSpacing: "1px",
        marginTop: "auto"
      }}>
        &copy; {new Date().getFullYear()} Alterion Test Dashboard
      </footer>
    </div>
  );
}