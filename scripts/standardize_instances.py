import sys
import os

def standardized_output_path(file_path: str) -> str:
    base, ext = os.path.splitext(file_path)
    return base + ".standardized" if ext else file_path + ".standardized"


def standardize_and_write(file_path):
    node_map = {}  # Map to hold node -> id mapping
    current_id = 0  # To assign new standardized IDs

    output_file_path = standardized_output_path(file_path)
    
    with open(file_path, 'r') as input_file, open(output_file_path, 'w') as output_file:
        output_file.write('#'*40+'\n')
        edges=set()
        loops=0
        duplicate_edges=0
    
        # konect bipartite networks have special format:
        # Both sides of bipartition are indexed starting at 1
        konect_bipartite = False 
        
        for line in input_file:
            line = line.strip()
            if '% bip' in line:
                konect_bipartite=True
            if not line or line[0]=='#' or line[0]=='n' or line[0]=='%':  # Skip empty lines
                continue
            # Parse edges (supports comma-separated or space-separated)
            if ',' in line:
                split = line.split(',')
            else:
                split = line.split()
            
            u=int(split[0])
            v=int(split[1])
            
            if konect_bipartite:
                v*=-1
            
            if u==v:
                loops+=1
                continue
            if (u < v and (u, v) in edges) or (u > v and (v, u) in edges):
                duplicate_edges+=1
                if duplicate_edges==1:
                    print(f"First duplicate edge: {u} {v}")
                continue
            edges.add((u,v) if u<v else (v,u))
            
    
            # Standardize node IDs
            if u not in node_map:
                node_map[u] = current_id
                current_id += 1
            if v not in node_map:
                node_map[v] = current_id
                current_id += 1
            
            output_file.write(f"{node_map[u]} {node_map[v]}\n")
        
        # Write stats at the beginning of the file
        output_file.seek(0)
        output_file.write(f"n {len(node_map)} m {len(edges)}\n")

    print(f"Found {loops} loops and {duplicate_edges} duplicate edges")
    print(f"Standardization complete. Output written to {output_file_path}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python standardize_instances.py <edge_list_file>")
        sys.exit(1)
    
    file_path = sys.argv[1]
    standardize_and_write(file_path)
